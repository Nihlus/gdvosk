// Copyright (C) 2024 Jarl Gullberg
// SPDX-License-Identifier: MIT

#include "semaphore_lock.h"

gdvosk::semaphore_lock::semaphore_lock(const godot::Ref<godot::Semaphore>& semaphore) :
    _semaphore(semaphore)
{
    _semaphore->wait();
}

gdvosk::semaphore_lock::~semaphore_lock()
{
    _semaphore->post();
}
