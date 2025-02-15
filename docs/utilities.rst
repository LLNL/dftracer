========================
DFTracer Utility scripts
========================

This section describes the utilities provided by DFTracer to assist users with logs.

----------

All scripts are installed with DFTracer in the installation's directories bin folder.

------------------
Merge Trace script
------------------

This script allows users to combine all pfw format into one.
This has the following signature.

.. code-block:: bash

    <install-dir>/bin/dftracer_merge [-fcv] [-d input_directory] [-o output_file]

Arguments for this script are

1. **-d input_directory** folder containing all trace files. Default `PWD`.
2. **-o output_file** file for storing merged file. Default `combined.pfw`.
3. **-f** override output file.
4. **-c** compress output file.
5. **-v** enable verbose mode.
6. **-h** display help

------------------
Compaction script
------------------

The script compacts all trace file and then divides the trace into equal file pieces.

.. code-block:: bash

    <install-dir>/bin/dftracer_compact [-fcv] [-d input_directory] [-o output_directory] [-l num_lines] [-p prefix]

Arguments for this script are

1. **-d input_directory** specify input directories. Should contain .pfw or .pfw.gz files. Default `PWD`.
2. **-o output_file** specify output directory. Default `combined.pfw`.
3. **-l num_lines** lines per trace.
4. **-p prefix** prefix to be used for compact files.
5. **-f** override output directory.
6. **-c** compress output file.
7. **-v** enable verbose mode.
8. **-h** display help

Alternatively, the **dftracer_compact_by_pid** script can be used to compact trace files by process ID using the same set of parameters.

------------------
Sanitize script
------------------

The script sanitizes all trace file to mask non-utf characters.

.. code-block:: bash

    <install-dir>/bin/usage: dftracer_sanitize [-fcv] [-d input_directory] [-o output_directory]

Arguments for this script are:

1. **-f** override output directory.
2. **-c** compress output file
3. **-v** enable verbose mode
4. **-h** display help
5. **-d input_directory** specify input directories. should contain .pfw or .pfw.gz files.
6. **-o output_directory** specify output directory.

------------------
Anonymize script
------------------

The removes system and file information from the trace and only maintain consistent hashes.

.. code-block:: bash

    <install-dir>/bin/usage: dftracer_anonymize [-fcv] [-d input_directory] [-o output_directory]

Arguments for this script are:

1. **-f** override output directory.
2. **-c** compress output file
3. **-v** enable verbose mode
4. **-h** display help
5. **-d input_directory** specify input directories. should contain .pfw or .pfw.gz files.
6. **-o output_directory** specify output directory.

------------------
Create Index script
------------------

The script compresses and creates index for all dftracer traces

.. code-block:: bash

    <install-dir>/bin/usage: dftracer_create_index [-fcv] [-d input_directory]

Arguments for this script are:

1. **-f** override indices.
2. **-c** compress input file
3. **-v** enable verbose mode
4. **-h** display help
5. **-d input_directory** specify input directories. should contain .pfw or .pfw.gz files.

------------------
Splitting DFTracer traces
------------------

The script splits the traces into equal sized chunk optimized for analysis

.. code-block:: bash

    <install-dir>/bin/dftracer_split [-fv] [-n app_name] [-d input_directory] [-o output_directory] [-s chunk_size]

Arguments for this script are:

1. **-n app_name** specify app name.
2. **-f** override indices.
3. **-s size** chunk size (in MB)
4. **-v** enable verbose mode
5. **-h** display help
6. **-d input_directory** specify input directories. should contain .pfw or .pfw.gz files.
7. **-o output_directory** specify output directory.

------------------
Counting DFTracer traces' events
------------------

The script will count number of valid events of traces

.. code-block:: bash

    <install-dir>/bin/dftracer_event_count [-f] [-d input_directory]

Arguments for this script are:

1. **-d input_directory** specify input directories. should contain .pfw or .pfw.gz files.
2. **-f**                 force index creation
3. **-h** display help

------------------
Validating DFTracer traces
------------------

The script validates dftracer traces

.. code-block:: bash

    <install-dir>/bin/dftracer_validate [-v] [-d input_directory]

Arguments for this script are:

1. **-d input_directory** specify input directories. should contain .pfw or .pfw.gz files.
3. **-h** display help
