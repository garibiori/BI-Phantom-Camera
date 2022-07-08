/**
 * @file Record.h
 * @author Ori Garibi
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
using namespace std;
using namespace Euresys;
//this class saves records of images
class Record{
    public:
        Record();
        Record(int index, uint64_t timeStamp, bool trig);
        Record(const Record& r1);
        ~Record();
        int index;
        uint64_t timeStamp;
        bool trig;
        
};
Record::Record(){

}
/**
 * @brief Construct a copy of a Record:: Record object
 * 
 * @param r1 
 */
Record::Record(const Record& r1){
    index = r1.index;
    timeStamp = r1.timeStamp;
    trig = r1.trig;
}
/**
 * @brief Construct a new Record:: Record object
 * 
 * @param index1 
 * @param timeStamp1 
 * @param trig1 
 */
Record::Record(int index1, uint64_t timeStamp1, bool trig1){
    index = index1;
    timeStamp = timeStamp1;
    trig = trig1;
}
Record::~Record(){

}
