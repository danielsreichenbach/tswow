/*
 * This file is part of tswow (https://github.com/tswow)
 * Copyright (C) 2025 tswow <https://github.com/tswow/>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <functional>
#include <utility>

namespace tswow::utils
{
    /**
     * RAII finalizer: runs the supplied callable when the object leaves
     * scope, unless dismiss() is called first.
     *
     *   tswow::utils::finally cleanup([&] { delete resource; });
     *   // ... code that may throw ...
     *   // resource is freed when `cleanup` goes out of scope.
     *
     * Move-constructible (so it can be returned from factories) but not
     * move-assignable, since rebinding an already-armed finalizer is
     * almost always a bug.
     */
    class finally
    {
    private:
        std::function<void()> m_finalizer;
        bool m_active;

    public:
        explicit finally(std::function<void()> finalizer)
            : m_finalizer(std::move(finalizer))
            , m_active(true)
        {
        }

        finally(const finally&) = delete;
        finally& operator=(const finally&) = delete;

        finally(finally&& other) noexcept
            : m_finalizer(std::move(other.m_finalizer))
            , m_active(other.m_active)
        {
            other.m_active = false;
        }

        finally& operator=(finally&&) = delete;

        ~finally()
        {
            if (m_active && m_finalizer)
            {
                m_finalizer();
            }
        }

        void dismiss() noexcept
        {
            m_active = false;
        }
    };
}
