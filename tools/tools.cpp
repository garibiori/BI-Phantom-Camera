#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <numeric>
#include <iomanip>
#include <algorithm>

#if defined(linux) || defined(__linux) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 600
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

#endif

#include "tools.h"
#include <D:\Euresys\eGrabber\include\internal\tools\EuresysLocks.h>

namespace Tools {

namespace {

#if defined(linux) || defined(__linux) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
static const char pathSeparator = '/';
void ensurePath(const std::string &path) {
    if (mkdir(path.c_str(), S_IRWXU|S_IRWXG|S_IRWXO)) {
        int err = errno;
        switch (err) {
            case EEXIST:
                break;
            default:
                throw std::runtime_error("ensurePath could not create " + path);
        }
    }
}
#else
static const char pathSeparator = '\\';
void ensurePath(const std::string &path) {
    if (!CreateDirectory(path.c_str(), NULL)) {
        DWORD lastError = GetLastError();
        switch (lastError) {
            case ERROR_ALREADY_EXISTS:
                break;
            case ERROR_PATH_NOT_FOUND:
            default:
                throw std::runtime_error("ensurePath could not create " + path);
        }
    }
}
#endif

std::string &getSamplesDirectory() {
    static std::string dir("samples");
    return dir;
}

Sample &getCurrentSample() {
    static Sample sample;
    return sample;
}

std::vector<Sample> &getSamples() {
    static std::vector<Sample> samples;
    return samples;
}

static const char *info[] = {
    "Euresys EGrabber Sample Programs",
    "--------------------------------",
    "",
    "Available samples programs:",
    0
};

static const char *help[] = {
    "Euresys EGrabber Sample Programs",
    "--------------------------------",
    "",
    "Command line arguments:",
    "  --run <sample>         run a sample from list below (substring match)",
    "  --runall               run all samples (except Specific ones)",
    "  --samples-dir <path>   set path to samples directory (default: samples)",
    "  --help                 display help",
    "  <no argument>          run interactive mode",
    "",
    "Available samples programs:",
    0
};

bool startsWith(const std::string &string, const std::string &prefix) {
    return string.find(prefix) == 0;
}

void showInfo(const char *info[]) {
    for (size_t i = 0; info[i]; ++i) {
        std::cout << info[i] << std::endl;
    }
    typedef std::vector<Sample>::const_iterator it_t;
    size_t nameLength = 0;
    std::map<std::string, std::vector<Sample> > categories;
    for (it_t it = getSamples().begin(); it != getSamples().end(); ++it) {
        if (nameLength < it->getDisplayName().size()) {
            nameLength = it->getDisplayName().size();
        }
        if (startsWith(it->getName(), "1")) {
            categories["Basic"].push_back(*it);
        } else if (startsWith(it->getName(), "2")) {
            categories["Intermediate"].push_back(*it);
        } else if (startsWith(it->getName(), "3")) {
            categories["Advanced"].push_back(*it);
        } else if (startsWith(it->getName(), "6")) {
            categories["Specific"].push_back(*it);
        } else {
            categories["Misc"].push_back(*it);
        }
    }
    std::vector<std::string> categoriesDisplayOrder;
    categoriesDisplayOrder.push_back("Basic");
    categoriesDisplayOrder.push_back("Intermediate");
    categoriesDisplayOrder.push_back("Advanced");
    categoriesDisplayOrder.push_back("Specific");
    categoriesDisplayOrder.push_back("Misc");
    for (size_t i = 0; i < categoriesDisplayOrder.size(); ++i) {
        const std::string &category(categoriesDisplayOrder[i]);
        const std::vector<Sample> &samples(categories[category]);
        if (!samples.empty()) {
            std::cout << spaces(2) << category << std::endl;
            for (it_t it = samples.begin(); it != samples.end(); ++it) {
                std::cout << spaces(4) << it->getDisplayName() << spaces(nameLength + 4 - it->getDisplayName().size());
                std::string tags = it->getDisplayTags();
                if (!tags.empty()) {
                    std::cout << tags << std::endl
                              << spaces(4 + nameLength + 4);
                }
                std::cout << it->getDescriptions()[0] << std::endl;
                for (size_t descIx = 1; descIx < it->getDescriptions().size(); ++descIx) {
                    std::cout << spaces(4 + nameLength + 4) << it->getDescriptions()[descIx] << std::endl;
                }
            }
        }
    }
    std::cout << std::endl;
}

void interaction() {
    showInfo(info);
    while (true) {
        std::cout << "Please enter the name of a sample to run: ";
        std::string sample;
        std::getline(std::cin, sample);
        if (sample.empty()) {
            showInfo(info);
            continue;
        } else if (sample == "q" || sample == "quit" || sample == "exit") {
            break;
        }
        try {
            run(sample);
        }
        catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
}

void runSample(const Sample &sample) {
    getCurrentSample() = sample;
    sample.run();
}

std::string joinAndEnsure2Path(const std::string &p1, const std::string &p2) {
    std::string path(join2Path(p1, p2));
    ensurePath(path);
    return path;
}

std::string joinAndEnsurePath(const std::vector<std::string> &paths) {
    return std::accumulate(paths.begin(), paths.end(), std::string(), joinAndEnsure2Path);
}

std::string getSampleOutputPath() {
    std::vector<std::string> xs;
    xs.push_back("output");
    xs.push_back(getCurrentSample().getName());
    return joinAndEnsurePath(xs);
}

std::string getEnvValue(const std::string &key) {
    if (key == "samples-dir") {
        return getSamplesDirectory();
    }
    if (key == "sample-output-path") {
        return getSampleOutputPath();
    }
    throw std::runtime_error("environment key not found: " + key);
}

std::string getSourceName(const std::string &path) {
    std::string p(path.substr(0, path.find_last_of(".")));
    size_t path_sep_pos = p.find_last_of("\\/");
    if (path_sep_pos == std::string::npos) {
        return p;
    }
    return p.substr(path_sep_pos + 1);
}

bool compareSamplesByName(const Sample &lhs, const Sample &rhs) {
    return lhs.getName() < rhs.getName();
}

void sortSamplesByName() {
    std::vector<Sample> &samples(getSamples());
    std::sort(samples.begin(), samples.end(), compareSamplesByName);
}

std::vector<std::string> splitDescription(const std::string &description) {
    std::stringstream ss(description);
    std::vector<std::string> lines;
    if (!description.empty()) {
        std::string line;
        while (std::getline(ss, line, '\n')) {
            lines.push_back(line);
        }
    }
    return lines;
}

} // anonymous

std::string spaces(size_t n) {
    return std::string(n, ' ');
}

#if defined(linux) || defined(__linux) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
void sleepMs(unsigned int ms) {
    if (usleep(1000 * ms)) {
        throw std::runtime_error("sleepMs failed");
    }
}
#else
void sleepMs(unsigned int ms) {
    ::Sleep(ms);
}
#endif

std::string join2Path(const std::string &p1, const std::string &p2) {
    if (p1.empty()) {
        return p2;
    }
    if (p1.at(p1.length() - 1) == pathSeparator) {
        return p1 + p2;
    }
    return p1 + pathSeparator + p2;
}

std::string getEnv(const std::string &key) {
    std::string value(getEnvValue(key));
    log("Tools::getEnv(\"" + key + "\")=" + value);
    return value;
}

std::string getSampleFilePath(const std::string &fileName) {
    return join2Path(getEnv("samples-dir"), fileName);
}

Sample::Sample(const std::string &source, sample_t fn, const std::string &description)
: name(getSourceName(source))
, descriptions(splitDescription(description))
, fn(fn)
, flags(0)
{
    getSamples().push_back(*this);
}

Sample::Sample(const std::string &source, sample_t fn, unsigned int flags, const std::string &description)
: name(getSourceName(source))
, descriptions(splitDescription(description))
, fn(fn)
, flags(flags)
{
    getSamples().push_back(*this);
}

Sample::Sample()
: fn(0)
, flags(0)
{}

Sample::Sample(const Sample &other)
: name(other.getName())
, descriptions(other.getDescriptions())
, fn(other.getFunction())
, flags(other.getFlags())
{}

DeprecatedSample::DeprecatedSample(const std::string &source, sample_t fn, const std::string &description)
: Sample(source, fn, DEPRECATED, description)
{}

std::string Sample::getName() const {
    return name;
}

std::string Sample::getDisplayName() const {
    std::stringstream ss;
    if (flags & DEPRECATED) {
        ss << "(" << name << ")";
    } else {
        ss << name;
    }
    return ss.str();
}

std::string Sample::getDisplayTags() const {
    std::stringstream ss;
    if (flags & DEPRECATED) {
        ss << "[DEPRECATED]";
    }
    return ss.str();
}

const std::vector<std::string> &Sample::getDescriptions() const {
    return descriptions;
}

sample_t Sample::getFunction() const {
    return fn;
}

unsigned int Sample::getFlags() const {
    return flags;
}

void Sample::run() const {
    log("Running \"" + name + "\"");
    if (fn) {
        try {
            fn();
        }
        catch (const std::exception &e) {
            std::string err("Exception while running sample \"" + name + "\": ");
            throw std::runtime_error(err + e.what());
        }
    } else {
        log("<null>");
    }
    log("Done.");
}

Sample &Sample::operator=(Sample rhs) {
    this->name = rhs.getName();
    this->descriptions = rhs.getDescriptions();
    this->fn = rhs.getFunction();
    this->flags = rhs.getFlags();
    return *this;
}

void runAll() {
    typedef std::vector<Sample>::const_iterator it_t;
    for (it_t it = getSamples().begin(); it != getSamples().end(); ++it) {
        if (startsWith(it->getName(), "6")) {
            log("Skipping \"" + it->getName() + "\"");
        } else {
            runSample(*it);
        }
    }
}

void run(const std::string &sample) {
    typedef std::vector<Sample>::const_iterator it_t;
    for (it_t it = getSamples().begin(); it != getSamples().end(); ++it) {
        if (it->getName().find(sample) != std::string::npos) {
            runSample(*it);
            return;
        }
    }
    throw std::runtime_error("sample \"" + sample + "\" not found");
}

void sample_main(int argc, char **argv) {
    std::map<std::string, std::string> params;
    std::vector<std::string> args(argv + 1, argv + argc);
    for (size_t i = 0; i < args.size();) {
        if (startsWith(args[i], "--")) {
            std::string name(args[i++].substr(2));
            std::string value;
            if (i < args.size() && !startsWith(args[i], "--")) {
                value.assign(args[i++]);
            }
            params[name] = value;
        } else {
            throw std::runtime_error("unexpected command line argument: " + args[i]);
        }
    }
    sortSamplesByName();
    // config
    if (params.count("samples-dir")) {
        getSamplesDirectory().assign(params["samples-dir"]);
        params.erase("samples-dir");
    }
    // commands
    if (params.count("runall")) {
        runAll();
    } else if (params.count("run")) {
        run(params["run"]);
    } else if (params.count("help")) {
        showInfo(help);
    } else if (params.size()) {
        throw std::runtime_error("unexpected command line argument: " + params.begin()->first);
    } else {
        interaction();
    }
}

void log(const std::string &msg) {
    static Euresys::Internal::ConcurrencyLock mutex;
    Euresys::Internal::AutoLock lock(mutex);
    std::cout << msg << std::endl;
}

std::string formatTimestamp(uint64_t timestamp) {
    std::stringstream ss;
    ss << std::setw(0) << (timestamp / 1000000) << "."
       << std::setw(6) << std::setfill('0') << (timestamp % 1000000);
    return ss.str();
}

#if defined(linux) || defined(__linux) || defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
uint64_t getTimestamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return ((uint64_t)ts.tv_sec * 1000000) + ts.tv_nsec / 1000;
}
#else
uint64_t getTimestamp()
{
    LARGE_INTEGER freqc, nowc;
    uint64_t freq, now;

    QueryPerformanceFrequency(&freqc);
    QueryPerformanceCounter(&nowc);
    freq = (uint64_t)freqc.QuadPart;
    now = (uint64_t)nowc.QuadPart;

    return (now / freq) * 1000000 + (now % freq) * 1000000 / freq;
}
#endif

}
