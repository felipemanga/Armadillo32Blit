#pragma once

#include <engine/file.hpp>
#include <cstddef>
#include <LibLog>
#include <optional>

struct FileInfo {
    std::string _name;
    const char *name() const {
        return _name.c_str();
    }
};

class File {
    std::optional<blit::File> handle;
    uint32_t pos = 0;
public:
    inline static int error;

    File() = default;
    File(const File&) = delete;

    File(File&& other) : handle{std::move(other.handle)}, pos{other.pos} {}

    File(const char* name, bool create = false, bool append = false) {
        openRW(name, create, append);
    }

    ~File(){
        close();
    }

    void close() {
        handle = std::nullopt;
        pos = 0;
    }

    operator bool(){
        return handle && handle->is_open();
    }

    static FileInfo stat(const char *name) {
        return FileInfo{blit::file_exists(name) ? name : ""};
    }

    File& openRO(const char *name) {
        close();
        handle.emplace(name, blit::OpenMode::read);
        if (!handle->is_open()) {
            handle = std::nullopt;
            error = 1;
        }
        return *this;
    }

    File& openRW(const char *name, bool create, bool append) {
        close();
        handle.emplace(name, blit::OpenMode::write);
        if (handle->is_open()) {
            handle = std::nullopt;
            error = 2;
        }
        return *this;
    }

    uint32_t size(){
        return handle->get_length();
    }

    uint32_t tell(){
        return pos;
    }

    File& seek(uint32_t offset){
        pos = offset;
        return *this;
    }

    uint32_t read( void *ptr, uint32_t count ){
        if(!*this) return 0;
        count = handle->read(pos, count, reinterpret_cast<char*>(ptr));
        pos += count;
        return count;
    }

    uint32_t write(const void *ptr, uint32_t count ){
        if(!*this) return 0;
        count = handle->write(pos, count, reinterpret_cast<const char*>(ptr));
        pos += count;
        return count;
    }

    template< typename T, size_t S > uint32_t read( T (&data)[S] ){
	    return read( data, sizeof(data) );
    }

    template< typename T, size_t S > uint32_t write( const T (&data)[S] ){
	    return write( data, sizeof(data) );
    }

    template< typename T >
    T read(){
        T tmp = {};
        read( (void*) &tmp, sizeof(T) );
        return tmp;
    }

    template< typename T >
    File & operator >> (T &i){
    	read(&i, sizeof(T));
    	return *this;
    }

    template< typename T >
    File & operator << (const T& i){
    	write(&i, sizeof(T));
    	return *this;
    }

    File & operator << (const char *str ){
        uint32_t len = 0;
        while(str[len]) len++;
        write(str, len);
    	return *this;
    }
};

