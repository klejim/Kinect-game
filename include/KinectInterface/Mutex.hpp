#pragma once
#ifndef SMARTMUTEX_HPP
#define SMARTMUTEX_HPP

#include <mutex> //std::mutex
#include <thread> //std::thread, std::thread::get_id(), std::thread::id()

/// \brief Un type de mutex capable de garder en mémoire le thread le possédant.
class smart_mutex : std::mutex
{
public:
    void lock()
    {
        // on appelle std::mutex::lock() puis on met à jour le propriétaire
        std::mutex::lock();
        m_holder = std::this_thread::get_id();
        return;
    }
    bool try_lock()
    {
        bool locked(false);
        if (std::mutex::try_lock())
        {
            m_holder = std::this_thread::get_id();
            locked = true;
        }
        return locked;
    }
    void unlock()
    {
        // on réinitialise la valeur du propriétaire puis déverrouille le mutex
        m_holder = std::thread::id();
        std::mutex::unlock();
        return;
    }

    bool is_locked_by_caller()
    {
        return m_holder == std::this_thread::get_id();
    }

private:
    std::thread::id m_holder = std::thread::id();
};
#endif