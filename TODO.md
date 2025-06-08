
# bigint

I may want to create a separate project for that one too, at some point,
although right now I have no need outside of the database.

However, the multiplication and division are both very slow and would need
to be fixed (i.e. use the more complex but much faster algorithms available).

Another library we should probably use is GMP, instead of our own:
https://gmplib.org/

# Languages

Whenever we do a query, we want to have a list of languages to query in a
specific order.

The concept is simple. If you ask for `fr_CA` and that specific French entry
does not exist, then try again with `fr`. If that fails, then try with any
region (i.e. `fr_FR`, `fr_BE`, etc.) If all of those fail, try again with
the same mechanism using the default language of the website. If that also
fails, return the first entry that exist in the database.

# Dependencies

## To be removed

We should be able to remove the dependency on OpenSSL since that library
should be used by eventdispatcher under the hood. Our library, at the
moment, leaks some #include of that library directly in their headers.

Once the eventdispatcher is fixed, we can fix prinbee as well by removing
the dependency.

## Installation

These need to be installed properly using the corresponding variables
(see the iplock project for example on how to do this).

* snaplogger

  To log messages (debug, info, errors, etc.)

* communicator

  For all network communications.

* fluid-settings

  For our settings which can be updated once for your entire cluster.

* as2js

  To define expressions for verification and indexing.

# coverage tests

* bigint/bigint -- done (`bigint`/`[u]int512[_t]`)
* bigint/round -- done
* data/convert -- done
* data/script -- 
* database/table -- 
* `file/file_snap_database_table` -- 
* block/... -- in progress [well, this will still be used, but only by indexes]
* data/structure -- in progress (fields are done, structure requires blocks & virtual blocks)
* pbql/... -- in progress

# query ideas

* Bigquery has an interesting instruction called `COUNTIF(predicate)` which
  could be useful in our environment; this is useful when querying a span of
  rows and you want to count how many times this or that happens along the
  way (although that probably requires a `GROUP BY` feature)
* Create support for a CREATE VIEW ... command and setup logical tables
  in that way. This allows for fast relational queries.

# prinbee-journal

* This tool is probably just a development tool and it should not be installed?
* The tool will apply a DELETE or TRUNCATE if that journal is setup that way;
  it seems that this should be a read-only tool... at the moment we cannot
  just change the parameter since that would change the .conf file

# Next Version

## Structure & Virtual Buffer

The virtual buffer and corresponding classes (such as the structure) should
be moved to a project of their own and moved under the eventdispatcher
project so that it can be used for bufferized data in our connections.

The data structure manager is practical to make use of binary blocks of
data which can easily be shared between processes and it is much more
advanced than the brs.h implementation (i.e. it allows for multiple versions
without having to save the field name for each field).

## BigInt

We should really consider using a library which has all the operations handy
and implemented in a really fast way. Although in our case, the database
mainly wants to read & write those large numbers as UUID, hashes, etc. so
no mathematical operations are required against them 99% of the time.

