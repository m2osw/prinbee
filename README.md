
# Introduction

The prinbee is an amorphic database system which allows us to manage
all of our data through one single system. It's 100% written in C++.

The basic features include:

* Replication -- data gets duplicated on multiple computers
* Horizontal Groth -- you can just add more computers to handle more data
* Resilience -- the network connections can be down and data is not lost


# Database Access

The library comes with a C++ library which allows clients to manage
the data from a very simple set of C++ objects. As far as the C++ user
is concerned, that's it.

## Circuit Breaker Feature

The client actually connects to a local proxy system working as a circuit
breaker. This pattern allows local applications to pretty much instantly
connect and get current information about the database cluster status.

This proxy system acts as a cache as well as a way to save data locally
before it gets sent to the cluster, making it really fast to do reads and
writes (as long as it is not the entire database). The cache is in part
in memory and in part on disk.

        +--------+
        |        |  <-- your application
        | Client |
        |        |
        +--------+
            ^
            | Unix Socket
    ========|============================================
            |
            v
       +-------------+
       |             |  <-- service running on the same computer as "Client"
       | Local Proxy |
       |             |
       +-------------+
            ^     ^
            |     +--------+
    ========|==============|================================
            |              |
            v              v
       +--------+       +--------+
       |        |       |        |  <-- connect to any number of nodes "Server"
       |  Node  |<----->|  Node  |
       |        |       |        |
       +--------+       +--------+

The graph shows the basic infrastructure.

Any computer that runs a database client must have the Local Proxy running
locally. That proxy is the one that connects to the Nodes. One Node sits on
one computer and in general that computer is only used to handle the database.

Node connect to each other to manage the data. Especially, the Nodes handle
replication in order to make sure you do not lose data.

## Cluster Object

The top object is the cluster object. This object represents a whole
database cluster. You may create any number of clusters.

A cluster comes with settings specific to the entire cluster.

## Context Object

The context represents one database environment with its set of tables.

A context comes with settings specific to the entire context.

## Table Object

Each context can include one or more tables. Although theorically you are
not limited in the number of tables in a context, the more tables you use
the larger the minimum amount of memory is required. Our systems currently
makes use of less than 50 tables and that works well on a set of 4Gb
computers.

A table comes with its own settings:

* Replication Factor

  How many times the data will be replicated. A duplicate always resides
  on another computer.

* Partitioning

  Whether partitioning is used--generally not necessary on tables that
  remain very small.

* Secure

  The table holds secure data. This means any row that gets deleted
  automatically gets cleared.

## Row Object

Whenever you run a `GET` command against a table you get rows as a result.

Rows handle the data by allowing you to set and get the values of each
column.

A row comes with its own settings:

* Time of Death

  Time when the row gets deleted automatically. When this feature is used
  we actually create an index on the _Time of Death_ field. This gives us
  a way to very quickly find the next rows to be deleted _automatically_.

### Mandatory Columns

All rows must have the following columns:

* Key

  There is no way to add the column to the table without a key.
  No other user data column is required. The key may include all
  the data the user needs. Keep in mind that the key may be composed
  of multiple columns

  The Key is used as the primary index on the table. If you do not need
  more than one index, then this is very efficient.

* Timestamp

  The date, in microseconds, when the row was created. This date is
  used to know which column to keep when doing a write (`SET`,
  `INSERT`, `UPDATE`). The one column we want to keep is the one
  with the largest timestamp (the newer one). All others are just
  _ignored_.

  The following is more or less an SQL equivalent of how the date
  gets used:

      UPDATE <table>
          SET <column> = <value>   ' repeat for all columns
          WHERE Key = key
            AND Timestamp > timestamp

  In other words, if the new timestamp is smaller or equal to the
  existing row timestamp, we ignore the write request (data is already
  too old).


# Packages Organization

The project manages several packages:

* `prinbee` -- The base package which includes the `prinbee` system user
               the library and a few other files.

* `prinbee-dev` -- The development files for the library.

* `prinbee-doc` -- The project and library documentation.

* `prinbee-proxy` -- The client's proxy, you install that along your clients.

* `prinbee-daemon` -- The daemon, this is the actual database manager.


# Bugs

Submit bug reports and patches on
[github](https://github.com/m2osw/prinbee/issues).


_This file is part of the [Snap! C++ project](https://snapwebsites.org/)._

vim: ts=4 sw=4 et
