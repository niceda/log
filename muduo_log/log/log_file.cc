// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)LogFile

#include "log_file.h"

#include "file_util.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>

LogFile::LogFile(const string &basename, off_t rollSize, bool threadSafe,
                 int flushInterval, int checkEveryN)
    : basename_(basename), rollSize_(rollSize), flushInterval_(flushInterval),
      checkEveryN_(checkEveryN), count_(0), startOfPeriod_(0), lastRoll_(0),
      lastFlush_(0) {
    assert(basename.find('/') == string::npos);
    rollFile();
}

LogFile::~LogFile() = default;

void LogFile::append(const char *logline, int len) {

    append_unlocked(logline, len);
}

void LogFile::flush() { file_->flush(); }

void LogFile::append_unlocked(const char *logline, int len) {
    file_->append(logline, len);

    if (file_->writtenBytes() > rollSize_) // 1.超过大小换文件
    {
        rollFile();
    } else {
        ++count_;       
        if (count_ >= checkEveryN_) { // 多次文件大小超过阈值才会走这个逻辑
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) {
                printf("%s(%d) startOfPeriod_: %s\n", __FUNCTION__, __LINE__, ctime(&startOfPeriod_));  // 学习观察用
                printf("%s(%d) thisPeriod_   : %s\n", __FUNCTION__, __LINE__, ctime(&thisPeriod_));  // 学习观察用
                rollFile();
            } else if (now - lastFlush_ > flushInterval_) {
                printf("%s(%d) flush(), now: %s\n", __FUNCTION__, __LINE__, ctime(&now));
                lastFlush_ = now;
                file_->flush(); // 超过一定的时间也主动刷新，比如不使用异步的时候
            }
        }
    }
}

bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;
    // printf("%s(%d) start    : %s\n", __FUNCTION__, __LINE__, ctime(&start));  // 学习观察用
    // printf("%s(%d) now      : %s\n", __FUNCTION__, __LINE__, ctime(&now));  // 学习观察用
    // printf("%s(%d) lastRoll_: %s\n", __FUNCTION__, __LINE__, ctime(&lastRoll_));  // 学习观察用
    if (now > lastRoll_) // 还有时间的计算，不是单纯按文件大小的，至少要过一秒才会换文件
    {
        // printf("%s(%d) start    : %s", __FUNCTION__, __LINE__, ctime(&start));  // 学习观察用
        // printf("%s(%d) now      : %s", __FUNCTION__, __LINE__, ctime(&now));  // 学习观察用
        // printf("%s(%d) lastRoll_: %s", __FUNCTION__, __LINE__, ctime(&lastRoll_));  // 学习观察用
        // printf("%s(%d) filename: %s\n\n", __FUNCTION__, __LINE__, filename.c_str());
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new FileUtil::AppendFile(filename));
        return true;
    }
    return false;
}

string LogFile::getLogFileName(const string &basename, time_t *now) {
    string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm); // FIXME: localtime_r ?
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.", &tm);
    filename += timebuf;

    filename += "log";

    return filename;
}
