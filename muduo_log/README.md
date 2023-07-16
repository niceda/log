# 项目简介


# 项目注意事项

## 不是超过文件大小就一定换文件
这里需要结合时间来确定是否换文件
```
bool LogFile::rollFile() {
    time_t now = 0;
    string filename = getLogFileName(basename_, &now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) // 还有时间的计算，不是单纯按文件大小的，至少要过一秒才会换文件
    {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new FileUtil::AppendFile(filename));
        return true;
    }
    return false;
}
```