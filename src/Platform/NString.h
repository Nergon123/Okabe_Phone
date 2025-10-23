#pragma once

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <string>

class NString {
  public:
    NString() : s_() {}
    NString(const char *c) : s_(c ? c : "") {}
    NString(const std::string &s) : s_(s) {}
    NString(char c) : s_(1, c) {}
    NString(int v) { s_ = std::to_string(v); }
    NString(unsigned int v) { s_ = std::to_string(v); }
    NString(long v) { s_ = std::to_string(v); }
    NString(unsigned long v) { s_ = std::to_string(v); }

    // Copy/move
    NString(const NString &)                = default;
    NString(NString &&) noexcept            = default;
    NString &operator=(const NString &)     = default;
    NString &operator=(NString &&) noexcept = default;

    // concatenation helpers
    NString &operator+=(const NString &o) {
        s_.append(o.s_);
        return *this;
    }
    NString &operator+=(const char *c) {
        if (c) { s_.append(c); }
        return *this;
    }
    NString &operator+=(char c) {
        s_.push_back(c);
        return *this;
    }

    friend NString operator+(const NString &a, const NString &b) { return NString(a.s_ + b.s_); }
    friend NString operator+(const NString &a, const char *b) {
        return NString(a.s_ + (b ? b : ""));
    }
    friend NString operator+(const char *a, const NString &b) {
        return NString((a ? a : "") + b.s_);
    }
    friend NString operator+(const NString &a, char b) {
        return NString(a.s_ + std::string(1, b));
    }

    // Add comparison operators after other friend operators
    friend bool operator==(const NString& a, const NString& b) { return a.s_ == b.s_; }
    friend bool operator==(const NString& a, const char* b) { return a.s_ == (b ? b : ""); }
    friend bool operator==(const char* a, const NString& b) { return (a ? a : "") == b.s_; }

    friend bool operator!=(const NString& a, const NString& b) { return !(a == b); }
    friend bool operator!=(const NString& a, const char* b) { return !(a == b); }
    friend bool operator!=(const char* a, const NString& b) { return !(a == b); }

    // length and emptiness
    unsigned long length() const { return (unsigned long)s_.size(); }
    bool          isEmpty() const { return s_.empty(); }

    // substring
    NString substring(ulong from, ulong to) const {
        if (from >= s_.size()) { return NString(); }
        if (to > s_.size()) { to = s_.size(); }
        if (to <= from) { return NString(); }
        return NString(s_.substr(from, to - from));
    }
    NString substring(unsigned long from) const {
        if (from >= s_.size()) { return NString(); }
        ulong to = s_.size();
        if (to <= from) { return NString(); }
        return NString(s_.substr(from, to - from));
    }

    // indexOf
    long indexOf(const NString &needle, unsigned long fromIndex = 0) const {
        size_t pos = s_.find(needle.s_, fromIndex);
        return pos == std::string::npos ? -1 : (long)pos;
    }
    long indexOf(const char *needle, unsigned long fromIndex = 0) const {
        if (!needle) { return -1; }
        size_t pos = s_.find(needle, fromIndex);
        return pos == std::string::npos ? -1 : (long)pos;
    }

    // Add lastIndexOf methods
    long lastIndexOf(const NString &needle, unsigned long fromIndex = std::string::npos) const {
        size_t pos = s_.rfind(needle.s_, fromIndex);
        return pos == std::string::npos ? -1 : (long)pos;
    }

    long lastIndexOf(const char *needle, unsigned long fromIndex = std::string::npos) const {
        if (!needle) { return -1; }
        size_t pos = s_.rfind(needle, fromIndex);
        return pos == std::string::npos ? -1 : (long)pos;
    }

    // replace
    void replace(const NString &target, const NString &replacement) {
        size_t pos = 0;
        while ((pos = s_.find(target.s_, pos)) != std::string::npos) {
            s_.replace(pos, target.s_.size(), replacement.s_);
            pos += replacement.s_.size();
        }
    }

    // trim
    void trim() {
        size_t start = 0;
        while (start < s_.size() && std::isspace((unsigned char)s_[start])) { ++start; }
        size_t end = s_.size();
        while (end > start && std::isspace((unsigned char)s_[end - 1])) { --end; }
        if (start == 0 && end == s_.size()) { return; }
        s_ = s_.substr(start, end - start);
    }

    NString toUpperCase() const {
        std::string upper = s_;
        for(char& c : upper) {
            c = std::toupper(static_cast<unsigned char>(c));
        }
        return NString(upper);
    }

    void remove(unsigned long index) {
        if (index >= s_.size()) {
            s_.clear();
            return;
        }
        s_.erase(index, 1);
    }

    int toInt() const { return std::atoi(s_.c_str()); }

    char charAt(unsigned long i) const { return i < s_.size() ? s_[i] : '\0'; }
    char operator[](unsigned long i) const { return charAt(i); }

    const char        *c_str() const { return s_.c_str(); }
                       operator std::string() const { return s_; }
    const std::string &stdstr() const { return s_; }

    void toCharArray(char *buf, unsigned int buflen) const {
        if (!buf || buflen == 0) { return; }
        size_t copy = std::min<size_t>(buflen - 1, s_.size());
        memcpy(buf, s_.data(), copy);
        buf[copy] = '\0';
    }

    bool startsWith(const NString &p) const { return s_.rfind(p.s_, 0) == 0; }
    bool endsWith(const NString &p) const {
        if (p.s_.size() > s_.size()) { return false; }
        return s_.compare(s_.size() - p.s_.size(), p.s_.size(), p.s_) == 0;
    }

    void        reserve(unsigned long n) { s_.reserve(n); }
    void        clear() { s_.clear(); }
    const char *c_str_safe() const { return s_.c_str(); }

  private:
    std::string s_;
};
