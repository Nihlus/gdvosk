// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef GDVOSK_SEMAPHORE_LOCK_H
#define GDVOSK_SEMAPHORE_LOCK_H

#include <godot_cpp/classes/semaphore.hpp>

namespace gdvosk
{
    class semaphore_lock final
    {
        godot::Ref<godot::Semaphore> _semaphore;

    public:
        explicit semaphore_lock(const godot::Ref<godot::Semaphore>& semaphore);
        ~semaphore_lock();

        // disable copy and move
        semaphore_lock(const semaphore_lock&) = delete;
        semaphore_lock(semaphore_lock&&) = delete;
        semaphore_lock& operator=(const semaphore_lock&) = delete;
        semaphore_lock& operator=(semaphore_lock&&) = delete;
    };
}

#endif //GDVOSK_SEMAPHORE_LOCK_H
