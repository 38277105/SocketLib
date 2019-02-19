/*
 * common.h
 *
 *  Created on: Dec 22, 2017
 *      Author: zj
 */

#ifndef COMMON_H_
#define COMMON_H_
#ifdef WIN32
#include<WinSock2.h>
#else
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<boost/thread/pthread/mutex.hpp>
#endif
#include<list>
#include<deque>
#include<map>
#include<set>
#include<mutex>
#include<boost/thread/thread.hpp>
//#include<thread/mutex.hpp>
#include<boost/thread/win32/mutex.hpp>
#include<boost/thread/lockable_adapter.hpp>
#include<boost/thread/locks.hpp>
#include<boost/thread/lock_guard.hpp>
#include<boost/pool/object_pool.hpp>
#include"typeDef.h"
#define IPLEN 20
#define CREATE_ERR -1
#define BIND_ERR -2
#define LISTEN_ERR -3
#define NULLSTR -1

#endif /* COMMON_H_ */
