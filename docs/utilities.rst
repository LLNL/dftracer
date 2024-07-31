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

    <install-dir>/bin/merge_pfw [-fcv] [-d input_directory] [-o output_file]

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
