#ifndef TOOLS_HEADER_FILE
#define TOOLS_HEADER_FILE

#include <iostream>
#include <sstream>
#include <iomanip>

#include <D:\Euresys\eGrabber\include\EGrabber.h>

namespace Tools {

typedef void (*sample_t)();

class Sample {
    public:
        Sample();
        Sample(const std::string &source, sample_t fn, const std::string &description);
        Sample(const Sample &other);
        std::string getName() const;
        std::string getDisplayName() const;
        std::string getDisplayTags() const;
        const std::vector<std::string> &getDescriptions() const;
        sample_t getFunction() const;
        unsigned int getFlags() const;
        void run() const;
        Sample &operator=(Sample rhs);
    protected:
        enum /* Flags */ {
            DEPRECATED = 1 << 0
        };
        Sample(const std::string &source, sample_t fn, unsigned int flags, const std::string &description);
    private:
        std::string name;
        std::vector<std::string> descriptions;
        sample_t fn;
        unsigned int flags;
};

class DeprecatedSample: public Sample {
    public:
        DeprecatedSample(const std::string &source, sample_t fn, const std::string &description);
        std::string fileName = "testing";
};

void runAll();
void run(const std::string &sample);
void sleepMs(unsigned int ms);
std::string getEnv(const std::string &key);
std::string getSampleFilePath(const std::string &fileName);
std::string join2Path(const std::string &p1, const std::string &p2);
void log(const std::string &msg);
std::string formatTimestamp(uint64_t timestamp);
uint64_t getTimestamp();
template <typename T> inline std::string toString(const T &v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
}
template <> inline std::string toString<std::string>(const std::string &v) {
    return v;
}
template <typename T> inline std::string toHexString(const T &v) {
    std::stringstream ss;
    ss << "0x" << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << v;
    return ss.str();
}
template <> inline std::string toHexString<std::string>(const std::string &v); // Not implemented
std::string spaces(size_t n);
void sample_main(int argc, char **argv);

template <unsigned char red, unsigned char green, unsigned char blue>
inline void fillRGB8(void *buffer, size_t size) {
    for (size_t i = 0; i < size / 3; ++i) {
        ((char *)buffer)[i*3 + 0] = red;
        ((char *)buffer)[i*3 + 1] = green;
        ((char *)buffer)[i*3 + 2] = blue;
    }
}

}

#endif
