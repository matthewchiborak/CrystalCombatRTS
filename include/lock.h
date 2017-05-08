/*
	Thread Locking Class (lock.h)

	Created: Jan 9, 2017
	Author: Daniel Bailey

	Description:
	Contains the lock class. If an instance of
	this class is passed to a series of threads,
	it can help ensure mutual excluion. It is
	recommended to implement the threads in a
	way that avoids starvation (Using sleeps).
*/

#ifndef __LOCK_H
#define __LOCK_H

/*
	A Lock is used to ensure that
	only one thread instance accesses
	memories at a time.
*/
class Lock
{
private:
	bool locked;
public:
	Lock() 
	{ 
		locked = false; 
	};

	const bool lock()
	{
		if (locked)
			return false;
		else
		{
			locked = true;
			return locked;
		}
	};
	void unlock()
	{
		locked = false;
	}
};


#endif