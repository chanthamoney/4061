＊ CSci4061 F2018 Assignment 2
＊ login: chant077
＊ date: 11/10/2018
＊ name: Cassandra Chanthamontry, Ounngy Ing, Jogey Vang
＊ id: chant077 (5216058), ingxx006 (5330616), vang2351 (5312382)

TODO: WE ASSUMED EMPTY MESSAGES CAN BE SENT
# Project 2: Multi-Process Chat App's Purpose

The purpose of this program is to develop a multi process chat app. There is a server
that can host up to 10 users/clients. The server and client can communicate together through
pipes. The server can use \list to list all of the users/clients, \kick <username> to terminate that specific
username's session, \exit to cleanup all the users and terminate their process, and send messages to the client.
The user can also use \list, \exit, and communicate to the server and all active users. It can also use \p2p <username> <message> to send a message to a particular user from the user.

## Who did what on the program

Cassandra Chanthamontry:
  Completed extra credit and command handlers in SERVER program

Ounngy Ing:
  Completed client program and error checking

Jogey Vang:
  Completed overall framework ,server program, extra credit.

### How to Compile the Program

In the directory where the program is located. Open terminal in that location and type in 'make' without the ''.
The only thing necessary for compiling the program is making sure the GNU makefile is located in the directory with the program files.

### How to use program from shell

First, execute the server with the command ./server. Then after the server is executed clients can be executed with ./client <username>. However, there can only be up to 10 clients and the clients should not have the same user name. Once these
programs (client and server) are executed messages can now be sent.

The server has 4 options.
  1. \list, which will list all the users/clients on the server.
  2. \kick <username>, which will kick the specified user off the chat session.
  3. \exit, which will terminate all the user sessions and close the chat server.
  4. <any-other-text>, the server can broadcast this message to all user process with the prefix "ADMIN-NOTICE". However,
  if there is no user, nothing will happen.

The client has 4 options.
  1. \list, which will list all the users/clients on the server in the user's process terminal window.
  2. \exit, which will disconnect this user, terminate the user process, and remove them from the chat session.
  3. \p2p <username><message>, which will send a personal message (<message>) to the user specified. However,
  it will print error if the user is invalid.
  4. <any-other-text>: Will broadcast this text to all user processes with the prefix of its username.
  5. \seg Will create segmantation fault in the user. However, this will not affect the server and other users as the server will kick the user out. 

## What exactly our program does

Our program implements the options listed above for the client and server. It creates a chat room,
where the user can communicate to the server or using \p2p, the user can communicate to a specified user. The server has the ability, to kick specified users out of the chat room as well as broadcast a message to all users. Both the server and client can list all the users in the chatroom and exit from the chat room as well.

### Explicit assumptions we have made

We assumed that the user and server can send empty messages to each other.
We assumed that when the server broadcast message to the users it will have the prefix, "ADMIN-NOTICE:"
due to inconsistency in the pdf.
We assumed that when a user leaves, a message will be broadcasted to remaining users and server that the user has left.
We assumed that when \p2p is called, the server will also be able to view that message as well.

### Strategies for error handling

We checked to make sure when we close a pipe to make sure that is closes by checking its return value.
We also checked the return value of all functions to make sure no error happens and when there is an error that
affects functionality, we terminate the whole program. However, if there's an error such like the user name does not exist
we simply output a statement stating that the User does not exist.
