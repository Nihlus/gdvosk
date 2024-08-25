// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#ifndef GDVOSK_SEMAPHORE_LOCK_H
#define GDVOSK_SEMAPHORE_LOCK_H

#include <godot_cpp/classes/semaphore.hpp>

namespace gdvosk
{
    /**
     * Provides a RAII-based interface for locking a Godot semaphore, similar to std::unique_lock.
     */
    class semaphore_lock final
    {
        /**
         * Holds a reference to the locked semaphore.
         */
        godot::Ref<godot::Semaphore> _semaphore;

    public:
        /**
         * Initializes a new instance of the semaphore_lock class, locking the given semaphore.
         * @param semaphore The semaphore to lock.
         */
        explicit semaphore_lock(const godot::Ref<godot::Semaphore>& semaphore);

        /**
         * Destroys an instance of the semaphore_lock class, releasing the locked semaphore.
         */
        ~semaphore_lock();

        // disable copy and move
        semaphore_lock(const semaphore_lock&) = delete;
        semaphore_lock(semaphore_lock&&) = delete;
        semaphore_lock& operator=(const semaphore_lock&) = delete;
        semaphore_lock& operator=(semaphore_lock&&) = delete;
    };
}

#endif //GDVOSK_SEMAPHORE_LOCK_H
