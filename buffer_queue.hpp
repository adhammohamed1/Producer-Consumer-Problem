#include <bits/stdc++.h>

using namespace std;

class Buffer {
    private:
        pair<short int, double> * bufptr; // Pointer to buffer segment
        int bufsize; // Buffer size
        int bufhead; // head of the buffer queue
        int buftail; // tail of the buffer queue
    
    public:

        /// @brief Buffer queue constructor
        Buffer( pair<short int, double> * __ptr, int __buffersize, int __head, int __tail )
        {
            bufptr = __ptr;
            bufsize = __buffersize;
            bufhead = __head;
            buftail = __tail;
        }

        /// @brief Setter function for the buffer head
        void set_head( int __head )
        {
            this->bufhead = __head;
        }

        /// @brief Getter function for the buffer head
        int get_head()
        {
            return this->bufhead;
        }

        /// @brief Setter function for the buffer tail 
        void set_tail( int __tail )
        {
            this->buftail = __tail;
        }

        /// @brief Getter function for the buffer tail
        int get_tail()
        {
            return this->buftail;
        }

        /// @brief This function checks if the buffer is empty or not 
        bool isempty()
        {
            return this->bufhead == this->buftail && this->bufptr[ this->buftail ].first == -1;
        }

        /// @brief This function checks if the buffer is full or not
        bool isfull()
        {
            return this->bufhead == this->buftail && this->bufptr[ this->buftail ].first > -1;
        }

        /// @brief Function to insert pair new_data into the buffer in FIFO policy.
        /// @return True  on success, false otherwise.
        bool insert( pair<short int, double> new_data )
        {
            if( this->isfull() ) return false;
            this->bufptr[ this->buftail ].first = new_data.first;
            this->bufptr[ this->buftail ].second = new_data.second;
            this->buftail = (this->buftail + 1) % this->bufsize;
            this->bufptr[ this->bufsize ].second = this->buftail;
            return true;
        }

        /// @return Value at the buffer_head
        pair<short int, double> front()
        {
            return this->bufptr[ this->bufhead ];
        }

        /// @brief This function is used to pop the element at the head
        /// @return True  on success, false otherwise.
        bool pop()
        {
            if( this->isempty() ) return false;
            this->bufptr[ this->bufhead ].first = -1;
            this->bufptr[ this->bufhead ].second = -1;
            this->bufhead = (this->bufhead + 1) % this->bufsize;
            this->bufptr[ this->bufsize ].first = this->bufhead;
            return true;
        }
};
