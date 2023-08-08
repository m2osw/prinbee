
# bigint

I may want to create a separate project for that one too, at some point,
although right now I have no need outside of the database.

However, the multiplication and division are both very slow and would need
to be fixed (i.e. use the more complex but much faster algorithms available).

Another library we should probably use is GMP:
https://gmplib.org/

# snaplogger / communicator / fluid-settings

These need to be installed properly using the corresponding variables
(see the iplock project for example on how to do this).

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
  count be useful in our environment; this is useful when querying a span of
  rows and you want to count how many times this or that happens along the
  way (although that probably requires a `GROUP BY` feature)

