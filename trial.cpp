/**
 * @file trial.cpp
 * @author Ori Garibi & Tian Lan
 * @brief 
 * @version 0.1
 * @date 2022-07-07
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "tools/tools.h"
#include <D:\Euresys\eGrabber\include\EGrabber.h>
#include <D:\Euresys\eGrabber\include\FormatConverter.h>
#include <iostream>
#include <direct.h>
#include <string>
#include <fstream>
#include "DoublyLinkedList.h"
#include "Record.h"

using namespace Euresys;     
using namespace std;

class MyGrabber : public EGrabber<CallbackOnDemand> {
    public:
        MyGrabber(EGenTL &gentl, int id, int trial, int numBuf, int bufferSize) : EGrabber<CallbackOnDemand>(gentl, id/2, id%2) { //initializing grabber class to set each grabber setting
            
            execute<DeviceModule>("DeviceReset");
            if(trial==1){ //set up only on the first trial
                if (id == 0) //master grabber
                {
                    setString<DeviceModule>("CameraControlMethod", "RC");  //master grabber set to RC, the rest set to NC
                    setString<InterfaceModule>("EventSelector", "LIN8");
                    setInteger<InterfaceModule>("EventNotification", true);
                    setString<InterfaceModule>("LineSelector", "TTLIO11");
                    setString<InterfaceModule>("LineMode", "Input");
                    setString<InterfaceModule>("LineInputToolSelector", "LIN8");
                    setString<InterfaceModule>("LineInputToolSource", "TTLIO11");
                    setString<InterfaceModule>("LineInputToolActivation", "RisingEdge");
                    
                    setString<RemoteModule>("TriggerMode", "TriggerModeOff");
                    setString<RemoteModule>("TriggerSource", "SWTRIGGER");
        
                    setString<RemoteModule>("Banks", "Banks_ABCD");
                    //setFloat<DeviceModule>("CycleMinimumPeriod",10000.0); // unit is uS. = 10e6/FPS

                    const int fps = 1000;

                    setInteger<RemoteModule>("AcquisitionFrameRate", fps);
                    setFloat<RemoteModule>("ExposureTime", 9e6/(fps*10)); //convert exposure time to 9e6/(fps*10)
                    enableEvent<IoToolboxData>();             
                }

                setString<StreamModule>("StripeArrangement", "Geometry_1X_2YM");
                setInteger<StreamModule>("LineWidth", 2560);
                setInteger<StreamModule>("LinePitch", 2560);
                setInteger<StreamModule>("StripeHeight", 4);
                setInteger<StreamModule>("StripePitch", 4);
                setInteger<StreamModule>("BlockHeight", 4);
                setInteger<StreamModule>("BufferPartCount", bufferSize);
                setInteger<StreamModule>("StripeOffset", 0);
                
            }
            int listSize = numBuf*bufferSize;

            reallocBuffers(numBuf); //reallocate buffers for each grabber
        }

    private:
        virtual void onIoToolboxEvent(const IoToolboxData &data) { //method to priunt data for specific frames
            std::cout << "timestamp: " << std::dec << data.timestamp << " us, "
                      << "numid: 0x" << std::hex << data.numid                  
                      << " (" << getEventDescription(data.numid) << "), "
                      << "Context1: " << data.context1 << ", "
                      << "Context2: " << data.context2 << ","
                      << std::endl;
        }
};


ofstream openFile(int trialCount){ //opens CSV file and inserts header
    ofstream timer;
    timer.open("D:/cameraOutput/Trial"+to_string(trialCount)+"/timeStamps_trial"+to_string(trialCount)+".csv");
    timer<<"Image Index"<<","<<"Timestamp(in microseconds)"<<","<<"Trigger"<<"\n";
    return timer;
}
void fileProcessor(ofstream &file, Record rec, int realIndex){ //prints data to CSV, data includes index, timestamp, and trigger
    file<<realIndex<<","<<rec.timeStamp<<","<<rec.trig<<"\n";
}
static void sample(int trialCount, int trial, int numBuf, int bufferSize, double concentration){
    DoublyLinkedList<uint8_t *> *imagePointer[4]; //DLL that stores image pointers for each grabber
    DoublyLinkedList<Record> *records = new DoublyLinkedList<Record>(); //DLL that stores image records
     
    for (int i =0; i<4; i++)
    {
        imagePointer[i] = new DoublyLinkedList<uint8_t *>();
    }

    
    IoToolboxData data;
    const int listSize = numBuf*bufferSize;
    ofstream timer = openFile(trialCount); //open file
    EGenTL genTL; // load GenTL producer
    MyGrabber* grabber[4]; //select the four grabbers

    for (int i=0; i<4; i++)
    {
        grabber[i] = new MyGrabber(genTL,i, trial, numBuf, bufferSize); // create grabber
    }

    for ( int i=3; i>-1; i--)
    {
        grabber[i]->start(); //start the four grabbers
    }

    FormatConverter converter(genTL); // create rgb converter environment
    int i = 0;
    bool trig = false;
    bool didTrigger = false;
    int numTrig = grabber[0]->getInteger<InterfaceModule>("EventCount[LIN8]");
    int halfList = listSize*concentration;

    for (size_t frame=0;frame < listSize; ++frame) { //start taking images
        if(frame >= halfList && didTrigger == false){ //if the concentration of the images have been taken and no trigger has been detected clear the back of the DLL for images and records
            for (int i=0; i <4; i++)
            {
                imagePointer[i]->removeBack();
            }
            records->removeBack();
            --frame; //go back a frame
        }
        trig = false;

        cout<<"Grabbing frame "<<frame<<endl;

        ScopedBuffer b0(*grabber[0]); // wait and get a buffer
        ScopedBuffer b1(*grabber[1]); // wait and get a buffer
        ScopedBuffer b2(*grabber[2]); // wait and get a buffer
        ScopedBuffer b3(*grabber[3]); // wait and get a buffer

        const size_t imgSize = b0.getInfo<size_t>(gc::BUFFER_INFO_SIZE);

        imagePointer[0]->insertFront(b0.getInfo<uint8_t *>(gc::BUFFER_INFO_BASE) + (frame%bufferSize)*imgSize/bufferSize); //grab images for each grabber
        imagePointer[1]->insertFront(b1.getInfo<uint8_t *>(gc::BUFFER_INFO_BASE) + (frame%bufferSize)*imgSize/bufferSize);
        imagePointer[2]->insertFront(b2.getInfo<uint8_t *>(gc::BUFFER_INFO_BASE) + (frame%bufferSize)*imgSize/bufferSize);
        imagePointer[3]->insertFront(b3.getInfo<uint8_t *>(gc::BUFFER_INFO_BASE) + (frame%bufferSize)*imgSize/bufferSize);
        
        uint64_t t0 = b0.getInfo<uint64_t>(gc::BUFFER_INFO_TIMESTAMP); //get each grabber's timestamp
        uint64_t t1 = b1.getInfo<uint64_t>(gc::BUFFER_INFO_TIMESTAMP);
        uint64_t t2 = b2.getInfo<uint64_t>(gc::BUFFER_INFO_TIMESTAMP);
        uint64_t t3 = b3.getInfo<uint64_t>(gc::BUFFER_INFO_TIMESTAMP);

        if(grabber[0]->getInteger<InterfaceModule>("EventCount[LIN8]") > numTrig && frame+1 >= halfList){ //trigger bools
            trig = true;
            didTrigger = true;
        }

        uint64_t tavg = (t0+t1+t2+t3)/4; //averave each grabber's timestamp
        Record record = Record(frame, tavg, trig);
        records->insertFront(record); //insert image record to list
    }
    
    const std::string format (grabber[0]->getPixelFormat()); //save image formats
    const size_t width = grabber[0]->getWidth();
    const size_t height = grabber[0]->getHeight();
    const size_t imgPitch = grabber[0]->getInteger<StreamModule>("LinePitch");
    const size_t imgSize = height*imgPitch;

    uint8_t * des = (uint8_t*) malloc (imgSize*4); //allocate destination memory for images

    for (size_t frames=0; frames<listSize; ++frames) { //begin saving
        cout<<"Saving frame "<<frames<<" to disk "<<endl;
        uint8_t* t[4];
        t[0]= imagePointer[0]->removeBack(); //remove each frame from the back of the DLL
        t[1]= imagePointer[1]->removeBack();
        t[2]= imagePointer[2]->removeBack();
        t[3]= imagePointer[3]->removeBack();

        uint8_t * tmp = des;
         for (int i=0; i<height/4; i++)            // top part loop through the whole image , each time copying 4 (subimages) *2 = 8 lines 
        {
            for (int j=3; j>-1; j--)               // loops through the 4 sub image 
            {
                memcpy(tmp,t[j],imgPitch*2);        // copy 2 lines from sub image j to current location of the pointer
                tmp +=  imgPitch*2;                 // move pointer forward by 2 lines
                t[j] += imgPitch*2;                 // move in the sub image the current pointer
            }
        }       
        
        for (int i=height/4; i<height/2; i++)      // bottom part loop through the whole image , each time copying 4 ( subimages) *2 = 8 lines 
        {
            for (int j=0; j<4; j++)                // loops through the 4 sub image 
            {
                memcpy(tmp,t[j],imgPitch*2);        // copy 2 lines from sub image j to current location of the pointer
                tmp += imgPitch*2;                   // move pointer forward by 2 lines
                t[j] += imgPitch*2;                 // move in the sub image the current pointer
            }
        }


        FormatConverter::Auto bgr(converter, FormatConverter::OutputFormat("RGB8"), des, format, width,  height * 4 * bufferSize, imgSize *4 *bufferSize, imgPitch);
            bgr.saveToDisk("D:/cameraOutput/Trial"+to_string(trialCount)+"/frame.NNN.jpeg", frames); //save stitched images
        fileProcessor(timer, records->removeBack(), frames); //assign index and write image data
    }
    free(des);
    timer.close(); //close file
}


int main(){
    //make it possible to change the before after ammount of images
    int numTrials = 6;
    int numBuf = 500;
    int bufferSize = 1;
    double concentration = 0.8;
    for(int trialCount = 1; trialCount <= numTrials; ++trialCount){ //run for certain ammount of trials
        string temp = "D:/cameraOutput/Trial" + to_string(trialCount); //create directory for images and files
        mkdir(temp.c_str());
        sample(trialCount, trialCount, numBuf, bufferSize, concentration);
    }
    return 0;
}