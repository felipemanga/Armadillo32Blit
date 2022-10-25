#pragma once

#ifdef TARGET_SDL2

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#elif TARGET_SDL1

#include <SDL.h>
#include <SDL_net.h>

#endif

#include <memory>
#include <variant>
#include <vector>
#include <unordered_map>

namespace sync {

    class Backend {
        class Property {
        public:
            void *address;
            std::unique_ptr<uptr[]> internal;
            std::size_t size;
            UpdateStrategy strategy;
        };

        class BackendEntity {
        public:
            Entity* entity;
            std::unordered_map<uptr, Property> properties;
        };

        class Connection {
            TCPsocket socket;

        public:
            Connection(IPaddress& ip) {
                connect(ip);
            }

            ~Connection() {
                if (socket)
                    SDLNet_TCP_Close(socket);
            }

            void connect(IPaddress& ip) {
                socket = SDLNet_TCP_Open(&ip);
                if (!socket) {
                    printf("SDLNet_TCP_Open: ", SDLNet_GetError(), "\n");
                    exit(2);
                }
            }

        };

        std::vector<std::unique_ptr<BackendEntity>> entities;
        IPaddress ip;

        std::variant<nullptr_t, Connection> connection;

        Backend() {
            connection = nullptr;

            int err;
            err = SDLNet_Init();
            if (err == -1) {
                LOG("Could not init SDLNet: ", SDLNet_GetError(), "\n");
                exit(1);
            }
            LOG("SDLNet started\n");
            if (SDLNet_ResolveHost(&ip, SYNC_SERVER, SYNC_PORT) == -1) {
                LOG("SDLNet_ResolveHost: ", SDLNet_GetError(), "\n");
                exit(1);
            }
            LOG("Sync server resolved\n");
        }

        BackendEntity& getBackend(Entity& entity) {
            for (auto& be : entities) {
                if (be->entity == &entity)
                    return *be;
            }
            entities.emplace_back(new BackendEntity{&entity});
            return *entities.back();
        }

    public:
        static Backend& instance() {
            static Backend obj;
            return obj;
        }

        void track(Entity& entity, uptr key, void* storage, std::size_t size, UpdateStrategy strategy) {
            BackendEntity& backend = getBackend(entity);

            u32 internalSize = size / sizeof(uptr);
            internalSize += internalSize * sizeof(uptr) < size;

            backend.properties[key] = Property {
                storage,
                std::make_unique<uptr[]>(internalSize),
                size,
                strategy
            };
        }
    };

}
