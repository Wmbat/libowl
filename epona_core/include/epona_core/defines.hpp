#pragma once

#define CORE_COPY_ONLY(name)                                                                       \
   name() = default;                                                                               \
   name(const name& other);                                                                        \
   name(name&& other) noexcept = delete;                                                           \
   ~name();                                                                                        \
                                                                                                   \
   name& operator=(const name& rhs);                                                               \
   name& operator=(name&& rhs) noexcept = delete;

#define CORE_MOVE_ONLY(name)                                                                       \
   name() = default;                                                                               \
   name(const name& other) = delete;                                                               \
   name(name&& other) noexcept;                                                                    \
   ~name();                                                                                        \
                                                                                                   \
   name& operator=(const name& rhs) = delete;                                                      \
   name& operator=(name&& rhs) noexcept;

#define CORE_COPY_MOVE(name)                                                                       \
   name() = default;                                                                               \
   name(const name& other);                                                                        \
   name(name&& other) noexcept;                                                                    \
   ~name();                                                                                        \
                                                                                                   \
   name& operator=(const name& rhs);                                                               \
   name& operator=(name&& rhs) noexcept;
