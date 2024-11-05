
# bigint

I may want to create a separate project for that one too, at some point,
although right now I have no need outside of the database.

However, the multiplication and division are both very slow and would need
to be fixed (i.e. use the more complex but much faster algorithms available).

Another library we should probably use is GMP:
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
* block/... -- in progress
* data/structure -- in progress (fields are done, structure requires blocks & virtual blocks)

# query ideas

* Bigquery has an interesting instruction called `COUNTIF(predicate)` which
  could be useful in our environment; this is useful when querying a span of
  rows and you want to count how many times this or that happens along the
  way (although that probably requires a `GROUP BY` feature)

# prinbee-journal

* This tool is probably just a development tool and it should not be installed?
* The tool will apply a DELETE or TRUNCATE if that journal is setup that way;
  it seems that this should be a read-only function... at the moment we cannot
  just change the parameter since that would change the .conf file

