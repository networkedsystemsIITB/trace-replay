# TraceReplay: Trace Based Application Layer Modeling in ns3

### Paper reference:

[Trace-based application layer modeling in ns3][PlDb], Prakash Agrawal and Mythili Vutukuru. Presented at Twenty-Second National Conference on Communications 2016.

### File structure:
The source code for TraceReplay is located in ``src/applications/model`` and consists of the following 6 files:
 - trace-replay-server.h,
 - trace-replay-server.cc,
 - trace-replay-client.h,
 - trace-replay-client.cc,
 - trace-replay-utility.h and
 - trace-replay-utility.cc

Helpers
*******
The helper code for TraceReplay is located in ``src/applications/helper`` and consists of the following 2 files:
 - trace-reaply-helper.h and 
 - trace-replay-helper.cc


Examples
********
The example for TraceReplay can be found at ``examples/trace-replay/trace-replay-example.cc``
### Installation:
To add TraceReplay to ns3 source code:

    * Add all the source files to corresponding directory
    
    * Add the class paths to src/applications/wscript
Run using waf
```sh
$ ./waf
```
To run TraceReplay example run following command:
```sh
$ ./waf --run "scratch/trace-replay-example --pcapPath=example/trace-replay/trace-replay-sample.pcap --nWifi=1"
```
    
    
   [PlDb]: <https://goo.gl/Z4ZW2K>
   

######Authors

- Prakash Agrawal, Master's student (2013-2016), Dept. of Computer Science and Engineering, IIT Bombay.
- Prof. Mythili Vutukuru, Dept. of Computer Science and Engineering, IIT Bombay.

######Contact

- Prakash Agrawal, prakash9752@gmail.com
- Prof. Mythili Vutukuru, mythili@cse.iitb.ac.in
