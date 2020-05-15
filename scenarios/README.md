<H1>MVM FW Scenario test results</H1>
<H2>as of: 15/05/2020 21:00</H2>
<Table>
<Tr><Td>Scenario Id</Td><Td>Description</Td><Td>Covered reqs</Td><Td>Coverage</Td><Td>Result</Td></Tr>
<Tr><Td><B> NBC_002</B></Td><Td>Normal behavior in PSV controlled mode</Td><Td>PSV.1, PSV.2</Td><Td>1378/2235 (   61 &percnt;)</Td><Td><B><Font color="red">FAILED</Font></B> ( - attribute g_mode not found in &lt;command&gt; event at t==9600 - attribute g_mode not found in &lt;command&gt; event at t==9600 - Test FAILED.)</Td></Tr>
<Tr><Td><B> NBC_001</B></Td><Td>Normal behavior in PCV controlled mode</Td><Td>PCV.1, PCV.2</Td><Td>1353/2235 (   60 &percnt;)</Td><Td><B><Font color="red">FAILED</Font></B> ( - found unwanted event for &lt;buzzer&gt; after t==2000, at t==5001 - found unwanted event for &lt;alarm_led&gt; after t==2000, at t==5001 - found unwanted event for &lt;alarm_relay&gt; after t==2000, at t==5001 - Test FAILED.)</Td></Tr>
<Tr><Td Colspan=3>TOTAL code coverage:</Td><Td>TOTAL coverage: 1381/2235 -  61.8%</Td><Td></Td></Tr>
</Table>
