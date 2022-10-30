

Important parts of Cassandra that we are using:

 * Load balancing by having X nodes running and sending further requests
   to a node that is not currently overwhelmed

   * Implementation wise, this is done by computing a hash (md5) to
     first index rows on specific computers; the cool result in this:
     the data gets sent to nodes X,Y,Z directly and this is very fast;
     the huge problem though, is adding and/or removing nodes; now we
     have data on various computers that should not be there and where
     it should be, it is not there yet... so that makes for a rather
     complicated algorithm to re-distribute the data quicly

     (Note: this hashing method is based on an indexed sequential
     exploit which is well known to DBMS implementers)

 * Replication capability having data sent to several nodes for "backup"
   and also faster retrieval (i.e. data can be retrieved from any computer
   that has a copy, not just the original)

   * Implementation wise, this works simply by sending the data to X
     nodes (X being the replication factor); this is generally done
     by sending the data to one node and then the node sending a copy
     of the data to the other X - 1 nodes

 * Simple database with (1) context [keyspace], (2) tables, (3) rows,
   (4) columns, (5) values, (6) time to live (TTL), (7) write timestamp.
   Nothing more.

   * Cassandra makes use of multiple contexts; we have to see the usefulness
     as it does not look like we would even want to have more than one for
     ourselves (although they have separate contexts to handle the system
     data)

   * Cassandra manages a TTL for each value; by default it is set to 'null'
     (undefined); we would prefer to use a specific date in the future
     because that way you do not have to use a TTL that could change between
     various cells (which it does right now with our Cassandra
     implementation!); we could still offer a TTL a la Cassandra which
     uses "time() + tll" calculated just at the time you save a cell.

   * The write timestamp tells us when that specific cell was written,
     which is useful to share between nodes (i.e. to know when the last
     write occurred)

   * The time to live and write timestamp are saved along each value
     in Cassandra; this is very practical as we can have one value such
     as a password::counter value which is used to count the number of
     times a user entered the wrong password but keep the counter only
     for a few hours; that counter can also be saved in the same
     row along other values which either do not have a TTL or have
     a different TTL... However, for a session, the entire row should
     be given a TTL and in many situations the TTL should be kept
     (i.e. a later INSERT without a USING TTL ### in Cassandra will
     lose the previous TTL since the new data is considered on its
     own; also Cassandra does not offer a TTL on a row, only on each
     cell / value.) So we want row and value TTLs that stick.

 * "Only columns are sorted." (rows are sorted, but using their MD5 sum
   so it is generally not viewed as a useful sort.) We created indexes
   using those columns in Cassandra.

      Note about MD5 -- Cassandra actually makes use of Murmur3 and not
      MD5 for their partitioner...
      https://en.wikipedia.org/wiki/MurmurHash
      https://github.com/aappleby/smhasher

   Note that in our case we actually want to create actual indexes so
   that way we can index the same set or rows in many different ways
   without the need to handle that by hand each time. More or less,
   we want the equivalent of an SQL "VIEW * FROM ... WHERE ...", with
   the WHERE being what the CREATE INDEX understands to sort a row and
   we want the sorting to be automatically updated whenever a write
   occurs.

   Ideas about sorting algorithms which could be useful to us?

      https://github.com/Morwenn/cpp-sort

   * Presence of data is checked using a Bloom filter; which is to keep a
     set of bits representing the result of a set of hash calculations.

     https://en.wikipedia.org/wiki/Bloom_filter

     https://hur.st/bloomfilter/

     We could look into it, but it seems to me that comparing long keys
     would be slower than having to compare the results of a very small
     number of hash results (i.e. say you have 4 hashes, you save there
     results in 4 bytes and compare those; if those do not even match
     then the key as a whole won't match either; i.e. we have to compute
     the hashes anyway to see whether the key has a chance to exist in
     the data store and if not ignore that part altogether, but if present
     the next step is to find the 4 bytes of hash and compare those...)

     Our bloom filter should make use of a fixed number of bits like 256.

     Cassandra seems to always use a bloom filter, when the number of
     items is really small, having to calculate the hash is probably
     not a good idea (i.e. if you have 3 rows, comparing them against
     the query key is probably much faster than having to calculate X
     hashes, check the bloom filter, get a positive answer and checking
     the 3 columns anyway...) However, it may be worth it if we can then
     check 4 bytes to find the rows, instead of the full row key (although
     we are likely to only get the MD5 on a GET order.)

     Note: Bloom filters need to be regenerated on a delete if you want
     to possibly remove some of the bits (it is not possible to just
     remove a bit). However, if we keep the 4 bytes of results, the
     "recalculation" should be really fast (outside of the large number
     of reads involved.) That is, we do not need to recalculate the hashes.

     Note: It looks like Cassandra keeps all sorts of bloom filter
     bit fields at various levels. That can also be used to save time
     to recalculate the filters. i.e. if you have an index store 256
     keys and have a Bloom filter field specific to these 256 keys,
     then you can first see whether the start/end match, if so, calculate
     the hashes, check that section Bloom filter, and know whether it
     exists or not. I would imagine that we calculate the hash at the
     start and check on a composite Bloom filter first:

     C++ bloom filter implementation: https://github.com/ArashPartow/bloom

     According to that and some research I have done, a good size
     is around 256 with 10 hashes for 1M elements. Assuming we have
     multiple levels, however, the precision increases and thus 4
     hashes and 2 or 3 levels of filtering would be way more than enough,
     without the need to spend any time calculating what would the best
     bloom filter be. (i.e. a hard coded super efficient version could be
     implemented; possibly using assembly directly--i.e. AVX offers 512
     bit registers...)

     1. How many rows?

       1.1 Less than X, then just search the rows really quick

       1.2 More than X, then calculate the hashes and check the Bloom
           filter; if all 1's, search for the block that matches
           (note that AVX offers an AND which works in 512 bits, so
           having a large number of bits here is slow because of hard
           drive access, but dead fast in memory.)

   * However, in most cases the fact that columns are sorted is not
     important; we could look into having three "types" of data added
     to a row: (see also talk about CREATE INDEX)

     1. sorted rows so one can create an index (the way to sort should
        also be a function--sort by number, ASCII, locale, time, where
        would a "null" go, etc. [can we have a column representing null
        in the first place?! not at this time]) -- in most cases we do
        not really need a specialized function, but for text sorted by
        a specific locale. We would need the user to define a "complex
        type" that describes the index key so the database can break
        it down as a set of items that can then be compared as expected.
        (i.e. the client can be offered to create keys using a class
        so each value added is properly typed, etc.)

     2. blob of X number of fields (something similar to what we do with
        the caching of some of our data, we transfer the whole set of fields
        in one go instead of many small fields, which takes forever,
        especially when the connection is encrypted) -- this should be
        100% transparent to the end user except for the fact that we need
        be able to tell which field can be in the blob and which cannot be
        (unless the system decides automatically using the size of the value)
        the "X" number could be managed by the system so once we reach a
        certain number, the system automatically breaks blobs in pieces
        (if I'm correct, this is what CQL offers with there CREATE TABLE
        since you may now create any number of "columns"; however, supporting
        a full fledge set of column names that would possibly be symetrical
        in all rows is not what we want at all.)

     3. regular fields that are "not sorted" (it still needs to be sorted
        but does not need to be an exact sort like (1) implies--i.e. this
        sort could use a hashed value which could better distribute the
        data between blocks within our index files.)

 * Consistency: the use of various types of consistencies (ONE, QUORUM,
   ALL are those we use. We should also look into supporting LOCAL_...
   once for the local DC opposed to remote DCs) This means we have to
   read/write the data in at least ONE, at least QUORUM, or ALL replication
   nodes. This means communication between nodes to make sure we have the
   latest data (i.e. a write may have happened on node 3 and that means
   a read from node 7 needs the data on node 3 since node 7 was not yet
   updated.)

   (i.e. If your replication factor is 3 and you have 12 nodes, ALL means
   the data will be saved on 3 nodes before the database says it is done
   with it. It will never copy the data on the 12 nodes.)

   An interesting read in resolving consistency in the real world:
   https://en.wikipedia.org/wiki/Paxos_(computer_science)
   However, for us, we probably want computers running the snapdatabase
   library/driver to assign the timestamp whenever it receives an order
   and not let the destination computers choose such (it is very important
   if we want to make sure we keep orders serialized. The C++ driver to
   handle Cassandra CQL does not do that so it NEVER guarantees which
   data arrives first when sending the same data twice to the same driver
   because when a node becomes overloaded, the driver auto-switch to another
   node and thus some data ends up being sent out of order and without our
   own TIMESTAMP info, the orders can end up being processed in the wrong
   order.)

   * Cassandra also offers a consistency of ZERO (write can fail) and
     a consistency of ANY (write can happen on any node, even if that's
     not a matching destination--i.e. token mismatch--although our own
     system would want to send the data to a matching node, if no such
     node is available, sending the data to a mismatched node would at
     least offer us a way to save that data in the database at some point.)

   * Implementation wise, the consistency means that we want to copy
     the data on Y nodes, where Y is 0, 1, 2, 3, QUORUM ((X + 1) / 2),
     or ALL; and that locally, rack wise, data center wise... Until
     all the writes happen (that many nodes say A-okay), we sit around...
     (note: Y = 0 is the ANY consistency meaning that we send the data
     to any one node and expect the write to somehow happen at some point)
     With QUORUM reads and QUORUM writes you get full consistency. Another
     way is to do writes with ALL and read of ONE, however, that will make
     for slow writes and the consistency is not as good as with QUORUM.

   * To implement the consistency (i.e. the replication property)
     you need to send the request to all the nodes that accept a similar
     hash; requests are always timestamped and we always keep the one with
     the largest timestamp, therefore the gossiping for replication
     first sends the exact hash and timestamp and if data is newer than
     what the other node has, it gets sent there.

   * Cassandra only keeps the latest version of any one cell (i.e cell
     with the largest timestamp); Bigtable is smarter and offers ways
     to read all the versions, just the latest, or any number of versions.
     In fact, it is capable of managing revisions and branches (that we
     currently do manually) in an automated way [although their mechanism
     does not allow us to manage our Snap! branches properly]. The
     Bigtable implementation is capable to delete all old data (only
     keey the most current cell), keep all the data, keep data by
     data (i.e. data older than 1 month gets deleted) and keep X
     number of entries (i.e. the last 3 revisions.) However, the garbage
     collectionof the data is per column-family and not per cell; so
     for that we would definitely need support for column-families...

   * Read and write of row data is atomic; this means only reads or
     one write can be happening at a time against a specific row;
     however, a write also has to make changes the index which also
     prevents reads from data around the same area

   * The Bigtable implementation supports atomicity over multiple
     write commands ("write" here means modifying); for example,
     the system can do INSERT, ("UPDATE"), DELETE in one atomic
     command by adding all the functions to a "row mutation"
     object and then apply all those operations in one atomic
     command; Cassandra uses the BATCH START INSERT/UPDATE ...
     BATCH APPLY; in CQL. It is not clear whether this really
     works between rows or even tables. For us, it would be great
     to be able to build a whole page in memory, including all
     its links, then APPLY that. This way our pages would atomically
     be correct (i.e. they get created or they fail being created.)

 * Gossip between nodes to maintain various information about the
   nodes (keep a list of existing and running nodes; would certainly
   be part of the load balancing work.)

   * Gossiping should be used for the following:
     . maintain information about capacity and load of nodes
     . do replication work (I got new data, there it is my friend)

 * Journaling, incoming data is first saved in a journal; this should allow
   us to save all the incoming data in a file used as a queue and once done
   with that queue, it gets deleted

 * With Cassandra CQL, the developers have started to use prepared
   statements (which I do not see the point because we should be limited
   to GET, SET, and DELETE, so one byte is more than enough to know what
   you want to do...), and also to create a VIEW (just like a la SQL,
   with the possibility to create new indexes and mix data between
   multiple tables); in effect the Cassandra VIEW is like an ongoing
   (INSERT INTO blah SELECT ...) type of statement since they actually
   create new data for the view so it performs at a descent (read normal
   full speed for Cassandra) speed; the Bigtable implementation includes
   some scripting capabilities (Sawzall and MapReduce) too that can be
   used on the server side to create various ongoing computations.
   These are not quite like a VIEW, but it could be viewed as being
   close. In our implementation, we have the pagelist that works
   very much like that (except our pagelist has write permissions!)
   This could be of interest: create a way to create lists from the
   actual server instead of a frontend computer.

 * Limitations of Cassandra

   * the main sorting capabilities are the partition
     (i.e. the mumur of row keys) and the columns. Both represent a key in
     a map, just like we have:

        table[row-key][column-key]

        Size of tables: max. size of one partition is limited to the amount of
        disk space available on the Cassandra node.

        A column value (blob) is at most 2Gb. Although remember that streaming
        that much data over the network is not going to be that fast.

        Collection values (TBD--sounds like this is the row key) cannot be more
        than 64Kb.

        Number of cells total in a partition is 2 billion (probably 2Gb). Here
        Cassandra sees a cell as Rows x Columns. I'm not too sure why there is
        a total limit instead of just 2Gb Rows x 2Gb Column.

        Counters are limited to a table, i.e. one table of just counter, or
        one table without any counters. We do not use nor really want to
        use counters at this point. If we are to do so, we should certainly
        look into using a specific type of table (like we would have a separate
        table to create work queues...)

    * the fact that the data is type less (see types above)

    * the fact that only one very specific type of tables is supported:
      big data tables. On our end we want to support several types:

        * "Big Data": few writes, many reads (nearly like immutable data)
        * Data: _regular data_, data that gets updated all the time but is
          still considered permanent
        * Temporary: _ephemerous data_, data which is like permanent data
          but will be deleted after a "small" while (i.e. data with a TTL)
        * Journals: a way to add data that our backends have to work on



What we use Cassandra for, but should not because these "activate"
anti-patterns:

 * Inter-front ends Lock capability. See snaplock [DONE]

 * Queues for backend processes. (The list, for example, adds paths
   to the listref, or some such table, so the pagelist backend can
   work on those pages once it wakes up and it is decided that the
   data is ready.)

   * Queues should use a completely separate implementation because
     the data is used once and then thrown away. There are three
     important points which we want to implement:

     - sort entries by keys (i.e. list entries have a priority and
       a date/time; must lists just have a date though)
     - files can have a limited size of about 1Mb after which we switch
       to another file; only keys need to remain sorted... so the
       newer file will only record keys after the last we had in the
       former file -- this means the 1Mb limit will break if new
       entries with a key lower than the existing last key are added;
       but that should not cause a problem assuming it does not happen
       indefinitely (i.e. if the key includes a date, it should always
       jump to the new files--yet we expect the jobs to all get worked
       on at least "once in a while" in which case all files can be
       deleted anyway...)
       [if we want to support never ending low keys, then we probably
       want to support two files, stop writing to the "old" file once
       we reach the limit and then run scans on the old and new files
       and return the smallest item from either file... as long as
       the items in the first file eventually all get worked on, it
       will get deleted at some point.]
     - deletions do not delete anything, once all the rows were deleted
       we can remove the file as a whole

   * Once all the jobs were completed, the file(s) get deleted since
     they are empty; this ensures that we do not just grow a file
     forever (we probably just want to reuse the file as is, without
     delete or truncation, unless we want to rescue some space on
     disk, in the event a file grew way big. Otherwise we can just
     overwrite the old data with new data.)

 * Local cache within libQtCassandra could be somehow replicated using
   our "proxy" front end server; i.e. send a "I have data from this
   timestamp" and the server can just say "you're good". This is very
   similar to getting a 304 from an HTTP server.

 * The idea of a CREATE INDEX ... command, as in SQL to get additional
   indexes on various tables. The way a bigtable works, we have columns
   that are sorted, so we can create an index as in:

     row key = "*index*"
     column key = "value from a certain 'normal' column"
     value = row key reference (i.e. a pointer to the actual data.)

   This is annoying because (1) now we have a special row in our table;
   and (2) the user has to manually add keys to the index.

   On our end, we could create indexes in an automated way. When creating
   a table with a PRIMARY KEY(col1, col2, ...), what the user is asking
   is to creates a composite column key, the concatenation of
   col1, col2, ... For that, we can offer a class that takes a set of
   data types and concatenates them. (As we do now with QByteArray and
   append...() functions.) So no need to have a PRIMARY KEY effort.
   We just want to read/write that data with an advance class available
   on the front end and not waste any time on the backend.

   The CREATE INDEX, on the other hand, tells us of another composite
   value used to sort the values that include a certain set of columns
   (TBD--how do we want to deal with Null data in that case? -- also
   what if the INDEX columns include one of the composite keys used
   in the PRIMARY KEY?) The system can then be responsible to find
   out whether any newly inserted or updated data (there is no updates
   in a bigtable, it's always an insert with a more recent timestamp)
   is part of that special key (INDEX) and if it matches "enough",
   just add that in an index. That index can be optimized compare to
   adding a standard row named '*index*' and then we have a row that
   could include thousands or even million of entries.
   [this is where the antipattern appears, although the users of
   Cassandra do not label Cassandra as such here...]

   Also, the way Cassandra works, those million of columns would all
   be saved on a number of nodes equal to the replication factor.
   And if you always call that index row '*index*', ALL those columns
   would end up on the ONE same set of Cassandra nodes. We probably
   want to look into a way to hash the column data so it gets at
   least partially distributed between all nodes (i.e. we cannot
   really use a hashing function for data being sorted, though.)

   The CREATE INDEX could include parameters that tell the system
   what kind of data to expect in the key (first few bytes, at
   least.) For example, you could say the key will start with an
   integer between X and Y, or a letter between 'a' and 'z', etc.


What the Cassandra cpp-driver offers that we would be using to ease
access to the cluster (i.e. this is not part of Cassandra servers):

 * Multi-backend connections, which is neat, but really it creates
   such connections in each child!!! This is not a good idea.


Somethings not offered by Cassandra:

 * "Proxy" server -- this is not currently true, that is, the driver
   itself is a standalone library but it should be relatively easy
   to have a proxy from that; the idea is that from the computer
   on which you run apps that have to access the Cluster, we need
   to have a proxy so that way we can have connections that are
   kept alive forever and a system that handles disconnect/reconnect
   automatically

 * Do permanent external backups (i.e. send a copy to another set of nodes
   that are there only to keep a copy of the data and not to participate
   in the cluster.)

 * "timed server"; Cassandra does not tell you whether the nodes and
   computers running the "Proxy server" are not synchronized time wise.
   This would be useful to make sure that the cluster works as expected
   over long period of time (i.e. if one clock drifts backward or fardward
   compare to the others, then it will possibly save data with the wrong
   timestamps if you do not run a proper timed service.)

 * Pre-compression on the client's node; depending on whether the network
   is super fast (i.e. no encryption between nodes) or slow (i.e. running
   with a VPN) we could ask the proxy to compress the data before sending
   it to the backend.

   Note: Cassandra uses compression on a per block manner; not only that
         they also use a checksum which is used mainly to make sure that
         if a read fails it gets detected and they avoid using that
         block entirely (the checksum is a simple CRC); I am wondering
         about this since (1) if data becomes invalid the XFS or whatever
         other file system should detect that and (2) per block means
         that we'd have data saved on a per block manner which we are
         not currently considering doing such; instead we are thinking
         to do it on a per row basis [although we could be hit if a
         table only has super small rows...]

 * Query using regex against keys; right now we have to get ALL the data
   from the server and scan it ourselves; which can be really slow if the
   network uses things such as tinc to encrypt the data; and especially if
   we want to match the row key against a regex since on the server side
   we can do that without having to read any of the column/value data!
   Cassandra forces us to read the columns and values of all the rows that
   may be a match (not even going to talk about the fact that Cassandra
   does not allow you to index against rows because we may not allow such
   either; actually, we may have totally flat data in one place and
   indexes in another!)

 * Testing whether the harddrives in use properly support fsync().
   (see http://brad.livejournal.com/2116715.html for the concept/details
   https://gist.github.com/bradfitz/3172656 for the .pl file)
   So... if writing to a file representing a database and we have to write
   many blocks to make sure that we are all good, we are not unlikely to
   overwrite data which could break table validity. We need to know how
   this is done in existing database systems so our implementation supports
   power outtages without data losses. (i.e. possibly read blocks A, B, C...
   save them in a temporary file of some sort with complete information
   of where to restore if required, do an fsync(). Then overwrite blocks
   A, B, C. Do fsync() on the new data and mark the saved data as not
   required anymore.)

   Note: we may be able to palliate the loss of power with the use of
         QUORUM writes for important data (which we never did with
         Cassandra and have had no concerns so far) assuming that
         all the computers will not all lose power all at once...
         (which in one physical rack is definitely not a guarantee!)

 * According to some documentation that I saw about Cassandra, the
   nodes form a ring and all the communications go through that ring.
   This can not only be really slow and "redundant," (i.e. data would
   get sent to the wrong nodes all the time) but if one node breaks
   down, it could take "forever" to get the cluster to communicate
   properly again. (As far as I know, the ring was not implemented
   in such a simple way, but I have not yet looked close enough to
   prove that one way or the other.)

   What we want is to have all the nodes that replicate the same
   data to be connected between each others. So, if you have a
   replication factor of 5, each node has 4 connections, one to
   each of the nodes that replicate its data. Similarly, we want
   many connections to other nodes with a rather large limit
   (probably around 100). At this time, I would think 1 connection
   per token range. So with 100 nodes and 5 as a replication
   factor, that would be 20 connections (on top of the 4 to
   quickly replicate the data.)

   By having connections to all the nodes that replicate a set
   of rows, the driver can send the data to any one of these
   nodes and that node will send a copy to all the other nodes
   that need to replace the data at once. By default the driver
   can be connected to at least one node per row range (i.e. if
   you have 100 nodes, a replication factor of 5, then you need
   20 connections from each driver... and that way each driver
   can send the data directly to a node that is to save that
   data.) However, we may also want to consider load balancing
   rather than just sending the data straight where it needs to
   go (but that could be a really busy computer and it could
   slow down things even more than sending it "the wrong node".)

   If the driver ("front end") did not compress the data, the
   first node that receives the data should do that first, then
   forward it to the other nodes (well "first" as in, just after
   it saved it in its own journal for processing by another thread.)

 * 
     This filter could be sent to the front end so a GET for a
     row can be checked on the front end and we get a ZERO network
     traffic if the row does not exist (except that the filter
     is not usable if any writes happen...)
     to at least check whether a new filter is available for
     that table... so a quick FILTER + date when we had the copy
     we currently have and get the 256 bytes or a 304... or do
     a push instead, after all one of each destinations is expected
     to have a connection to each front end... TBD) At the same
     time, a row query calculates the MD5 upfront and sending the
     MD5 is very small. On a read we do not need the full key!


More or less, the concept of the file structure we want to use:

 * The context and table names are limited to lowercase,
   digits and underscores. The name cannot start or end
   with an underscore. It also cannot start with a digit
   and it must be at least two characters:
     
         [a-z][a-z0-9_]*[a-z0-9]

 * File handling, why using the block size can be useful?
   (i.e. a block size is generally 4096 on Linux, this can
   be changed, though, thus we need to get the disk defined
   block size!)

   There is a command named fallocate() which can be used to
   allocate/diallocate (mark the file sparse) on a per block
   basis. This can be really useful since a file with many
   "holes" can immediately be deallocated, bit by bit...

   The allocation feature can be used to make sure that space
   is available on disk to write the final data (i.e. if you
   need 3 blocks, call fallocate() with offset equal to EOF
   and the length as 3 x 4096 and if that fails, a write
   with the data would fail anyway...)

   Check out `man 2 fallocate` (or just `man fallocate`.)

 * Directory structure:

   - /var/lib/snapdb

     Root where all the files get saved.

   - /var/lib/snapdb/_info.def

     Information about this cluster, such as its name.

   - /var/lib/snapdb/<context>

     We have one directory per context. The name is the context
     name itself. A command to list all contexts is just a glob()
     over this directory and ignore files that start with an
     underscore.
 
   - /var/lib/snapdb/<context>/_info.def

     Information about this context, such as the replication factor.

   - /var/lib/snapdb/<context>/<table>

     We have one directory per table. The name is the table name
     itself. A command to list all tables of a context is just a
     glob() over the context directory and ignore files that start
     with an underscore.

   - /var/lib/snapdb/<context>/<table>/_info.def

     Information about this table, such as the compaction and
     compression methods.

     The info also includes a Bloom filter of exactly 256 bits.

   - /var/lib/snapdb/<context>/<table>/_data-1.bin

     A data file (name to be strengthen.)

     Writes (SET) always append new data to such files. "Later"
     the compaction process scans these files and removes the
     data that was updated or dropped. (i.e. an update does not
     delete the old data, it just appends at the end and then
     mark the old data space as "deleted"--our algorithm  should
     be smart enough to overwrite if there is enough space,
     although we want to do that in a totally safe manner and
     in most cases an overwrite is not that safe...)

     Also, a SET in Cassandra saves the few new columns specified,
     not one row. That results in sparse rows! (i.e. you may have
     to read 100 blocks if you have a row with 100 columns.)
     Although the compaction process will, over time, regroup these
     columns together. (This is certainly fine when we create
     indexes with those columns, but not for our "standard" data.)

   - /var/lib/snapdb/<context>/<table>/_index.def

     Information about indexes defined over this table.

   - /var/lib/snapdb/<context>/<table>/_index-1.bin

     An index file (each index has [at least] one such file.)

   - /var/lib/snapdb/<context>/<view>/_info.def

     A view definition, this is similar to an index, but it is
     capable of indexing more than one table, as a result it
     creates a view.

   - /var/lib/snapdb/<context>/<view>/_index-1.bin

     An index file (this is the actual view index.)


vim: ts=4 sw=4 et
