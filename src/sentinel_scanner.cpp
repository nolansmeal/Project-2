#include "core/sentinel_scanner.h"

SentinelScanner::SentinelScanner(string sentinel) {
    sentinel_ = sentinel;
    pending_ = "";
}

SentinelScanner::Out SentinelScanner::feed(string_view chunk) {
    string text = pending_ + string(chunk);
    pending_ = "";

    size_t position = text.find(sentinel_);

    if (position != string::npos) {
        string safe = text.substr(0, position);
        return {safe, true};
    }

    size_t keep = sentinel_.size() - 1;

    if (text.size() <= keep) {
        pending_ = text;
        return {"", false};
    }

    size_t safe_size = text.size() - keep;

    string safe = text.substr(0, safe_size);
    pending_ = text.substr(safe_size);

    return {safe, false};
}

SentinelScanner::Out SentinelScanner::flush() {
    string safe = pending_;
    pending_ = "";

    return {safe, false};
}