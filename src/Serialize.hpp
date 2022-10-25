#pragma once

#include <File>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <string>
#endif

using SerializeProperty = void (*)(const StringInfo&, void*, std::size_t);

class Serialize {
    using Proxy = void (*)(void* obj, SerializeProperty cb);
    u32 hash, size;
    File file;
    inline static Serialize* instance;
    inline static bool wasInit = false;

    Serialize() {
        init();
    }

public:
    static void init(){
        if (wasInit)
            return;
        wasInit = true;

        #ifdef __EMSCRIPTEN__
        EM_ASM(
            FS.mkdir("/persistent");
            FS.mount(IDBFS, {}, "/persistent");
            FS.syncfs(true, function (err) {
                    try{ FS.mkdir("/persistent/data"); } catch(ex) {}
                });
            );
        #endif
    }

    bool toFile(void* value, Proxy proxy, const char* fileName) {
        #ifdef __EMSCRIPTEN__
        std::string path = "/persistent/";
        path += fileName;
        fileName = path.c_str();
        #endif

        if (!file.openRW(fileName, true, false)) {
            LOGD("Could not open ", fileName, "\n");
            return false;
        }

        proxy(value, +[](const StringInfo& info, void* ptr, std::size_t size){
            instance->file << (u32) info << (u32) size;
            instance->file.write(ptr, size);
            // LOG("Saved ", (std::string_view)info, "\n");
        });

        #ifdef __EMSCRIPTEN__
        EM_ASM(
            FS.syncfs(function (err){
                    // console.log("Sync'ed");
                });
            );
        #endif
        return true;
    }

    template <typename Type>
    static bool toFile(Type& value, const char* fileName) {
        Serialize s;
        instance = &s;
        auto proxy = +[](void* obj, SerializeProperty cb){
            reinterpret_cast<Type*>(obj)->_serialize_(cb);
        };
        return s.toFile(reinterpret_cast<void*>(&value), proxy, fileName);
    }

    bool fromFile(void* value, Proxy proxy, const char* fileName) {
        #ifdef __EMSCRIPTEN__
        std::string path = "/persistent/";
        path += fileName;
        fileName = path.c_str();
        #endif

        if (!file.openRO(fileName))
            return false;

        while (true) {
            size = 0;
            file >> hash >> size;
            if (!size) return true;
            proxy(value, +[](const StringInfo& info, void* ptr, std::size_t size){
                if (info == instance->hash) {
                    instance->file.read(ptr, std::min<u32>(instance->size, size));
                    // LOG("Loaded ", (std::string_view)info, " ", instance->file.tell(), " ", instance->size, "\n");
                    instance->file >> instance->hash >> instance->size;
                }
            });
        }
    }

    template <typename Type>
    static bool fromFile(Type& value, const char* fileName) {
        Serialize s;
        instance = &s;
        auto proxy = +[](void* obj, SerializeProperty cb){
            reinterpret_cast<Type*>(obj)->_serialize_(cb);
        };
        return s.fromFile(reinterpret_cast<void*>(&value), proxy, fileName);
    }
};

#define SERIALIZE(FILENAME)                                                             \
    bool save(const char* name = FILENAME) { return Serialize::toFile(*this, name); }   \
    bool load(const char* name = FILENAME) { return Serialize::fromFile(*this, name); } \
    void _serialize_(SerializeProperty cb_)

#define PROPERTY(NAME) cb_( #NAME, reinterpret_cast<void*>(&NAME), sizeof(NAME) )
