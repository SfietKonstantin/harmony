# Harmony project

The Harmony project is an experiment about using your Linux (SailfishOS) based phone from a client.
This client can be another phone, a tablet, or a computer.

Harmony is composed of two components:
- A daemon, written in C++, that serves information from the phone. It interacts with the 
  middleware and uses an embedded webserver to expose those information. Reading from the
  middleware is done with plugins.
- A web app, that can be opened on any modern browser, that is the client interface.

Optionally, the web app can be replaced by a desktop or mobile native application. This application
can use the same RESTful APIs as the web app.

## Writing a plugin
