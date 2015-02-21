# Harmony project

The Harmony project is an experiment about using your Linux (SailfishOS) based phone from a client.
This client can be another phone, a tablet, or a computer.

Harmony is composed of three components:
- A daemon, written in C++, that interacts with the middleware and is used to expose information
  to DBus. This daemon uses plugins to provide specific capabilities.
- A server running on top of NodeJS, that is a thin layer between the phone and the client. This
  server translates the DBus API to a REST-like API.
- A web app, that can be opened on any modern browser, that is the client interface.

## Writing a plugin
