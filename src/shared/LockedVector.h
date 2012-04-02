/*
 * Copyright (C) 2009-2012 /dev/rsa for MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* based on LockedQueue class from MaNGOS */
/* written for use instead not locked std::list && std::vector */

#ifndef LOCKEDVECTOR_H
#define LOCKEDVECTOR_H

#include <ace/RW_Thread_Mutex.h>
#include <vector>
#include <assert.h>
#include "Errors.h"

namespace ACE_Based
{
    template <class T, typename StorageType     = std::vector<T> >
    class LockedVector
    {

        public:

        typedef   ACE_RW_Thread_Mutex          LockType;
        typedef   ACE_Read_Guard<LockType>     ReadGuard;
        typedef   ACE_Write_Guard<LockType>    WriteGuard;
        typedef   StorageType                  TVector;
        typedef   T                            vType;
        typedef   vType*                       pointer;
        typedef   const vType*                 const_pointer;
// need #ifdef for VC
        typedef __gnu_cxx::__normal_iterator<pointer, TVector>       iterator;
        typedef __gnu_cxx::__normal_iterator<const_pointer, TVector> const_iterator;

        typedef std::reverse_iterator<const_iterator>                const_reverse_iterator;
        typedef std::reverse_iterator<iterator>                      reverse_iterator;
        typedef vType&                         reference;
        typedef const vType&                   const_reference;
        typedef size_t                         size_type;

        public:

            LockedVector()
            {
            }

            virtual ~LockedVector()
            {
                WriteGuard Guard(GetLock());
            }

            void push_back(const T& item)
            {
                WriteGuard Guard(GetLock());
                m_vector.push_back(item);
            }

            void pop_back()
            {
                WriteGuard Guard(GetLock());
                m_vector.pop_back();
            }

            void erase(size_t pos)
            {
                WriteGuard Guard(GetLock());
                m_vector.erase(m_vector.begin() + pos);
            }

            void remove(const T& item)
            {
                erase(item);
            }

            void erase(const T& item)
            {
                WriteGuard Guard(GetLock());
                for (int i = 0; i < m_vector.size();)
                {
                    if (item == m_vector[i])
                        m_vector.erase(m_vector.begin() + i);
                    else
                        ++i;
                }
            }

            void find(const T& item)
            {
                ReadGuard Guard(GetLock());
                for (int i = 0; i < m_vector.size();)
                {
                    if (item == m_vector[i])
                        return &m_vector[i];
                }
            }

            void clear()
            {
                WriteGuard Guard(GetLock());
                m_vector.clear();
            }

            T& operator[](size_t idx) 
            {
                ReadGuard Guard(GetLock());
                if (idx >= m_vector.size() || idx < 0)
                    return NULL;
                return m_vector[idx];
            }

            T& front()
            {
                ReadGuard Guard(GetLock());
                return m_vector.front();
            }

            iterator begin()
            {
                ReadGuard Guard(GetLock());
                return m_vector.begin();
            }

            iterator end()
            {
                ReadGuard Guard(GetLock());
                return m_vector.end();
            }

            const_iterator begin() const
            {
                ReadGuard Guard(GetLock());
                return m_vector.begin();
            }

            const_iterator end() const
            {
                ReadGuard Guard(GetLock());
                return m_vector.end();
            }

            const_reverse_iterator rbegin() const
            {
                ReadGuard Guard(GetLock());
                return m_vector.rbegin();
            }

            const_reverse_iterator rend() const
            {
                ReadGuard Guard(GetLock());
                return m_vector.rend();
            }

            bool empty() const
            {
                ReadGuard Guard(GetLock());
                return    m_vector.empty();
            }

            size_t size() const
            {
                ReadGuard Guard(GetLock());
                return    m_vector.size();
            }

            LockType&       GetLock() { return i_lock; }
            LockType&       GetLock() const { return i_lock; }

            LockedVector&    operator=(const StorageType &v)
            {
                WriteGuard Guard(GetLock());
                for (typename StorageType::const_iterator i = v.begin(); i != v.end(); ++i) 
                {
                    this->push_back(*i);
                }
                return *this;
            }

            LockedVector(const StorageType &v)
            {
                WriteGuard Guard(GetLock());
                for (typename StorageType::const_iterator i = v.begin(); i != v.end(); ++i) 
                {
                    this->push_back(*i);
                }
            }

            LockedVector&    operator=(const LockedVector<T> &v)
            {
                WriteGuard Guard(GetLock());
                for (typename LockedVector<T>::const_iterator i = v.begin(); i != v.end(); ++i) 
                {
                    this->push_back(*i);
                }
                return *this;
            }

            LockedVector(const LockedVector<T> &v)
            {
                WriteGuard Guard(GetLock());
                for (typename LockedVector<T>::const_iterator i = v.begin(); i != v.end(); ++i) 
                {
                    this->push_back(*i);
                }
            }

            template <typename C>
            void sort(C& compare)
            {
                iterator _begin = begin();
                iterator _end   = end();
                WriteGuard Guard(GetLock());
                std::stable_sort(_begin,_end,compare);
            }

        protected:
            mutable LockType       i_lock;
            StorageType            m_vector;
    };
}
#endif
