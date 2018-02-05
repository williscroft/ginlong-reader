Monitor a Ginlong wifi inverter data logger.

You will need a a Ginlong inverter for this to do anything useful.

Connect an analog temperature sensor to AN0 to get a reading for temperature.

Fill in wifi credentials etc.

Configure the inverter logger 
Advanced->Remote server
server 1 defaults to Ginlongmontoring.com in china.
Set server 2
server 2 <ip address> 3030 UDP.

The UDP format is very nearly the same as the TCP streaming format,
just easier to pick up message boundaries.

See https://github.com/graham0/ginlong-wifi for tcp stream parser.

