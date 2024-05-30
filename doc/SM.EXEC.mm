<map version="freeplane 1.3.0">
<!--To view this file, download free mind mapping software Freeplane from http://freeplane.sourceforge.net -->
<node TEXT="SM.EXEC" LOCALIZED_STYLE_REF="styles.topic" ID="ID_1723255651" CREATED="1283093380553" MODIFIED="1539637272688" COLOR="#000000" STYLE="bubble">
<icon BUILTIN="launch"/>
<hook NAME="MapStyle">
    <properties show_note_icons="true"/>

<map_styles>
<stylenode LOCALIZED_TEXT="styles.root_node">
<stylenode LOCALIZED_TEXT="styles.predefined" POSITION="right">
<stylenode LOCALIZED_TEXT="default" MAX_WIDTH="600" COLOR="#000000" STYLE="as_parent">
<font NAME="SansSerif" SIZE="10" BOLD="false" ITALIC="false"/>
</stylenode>
<stylenode LOCALIZED_TEXT="defaultstyle.details"/>
<stylenode LOCALIZED_TEXT="defaultstyle.note"/>
<stylenode LOCALIZED_TEXT="defaultstyle.floating">
<edge STYLE="hide_edge"/>
<cloud COLOR="#f0f0f0" SHAPE="ROUND_RECT"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.user-defined" POSITION="right">
<stylenode LOCALIZED_TEXT="styles.topic" COLOR="#18898b" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subtopic" COLOR="#cc3300" STYLE="fork">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.subsubtopic" COLOR="#669900">
<font NAME="Liberation Sans" SIZE="10" BOLD="true"/>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.important">
<icon BUILTIN="yes"/>
</stylenode>
</stylenode>
<stylenode LOCALIZED_TEXT="styles.AutomaticLayout" POSITION="right">
<stylenode LOCALIZED_TEXT="AutomaticLayout.level.root" COLOR="#000000">
<font SIZE="18"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,1" COLOR="#0033ff">
<font SIZE="16"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,2" COLOR="#00b439">
<font SIZE="14"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,3" COLOR="#990000">
<font SIZE="12"/>
</stylenode>
<stylenode LOCALIZED_TEXT="AutomaticLayout.level,4" COLOR="#111111">
<font SIZE="10"/>
</stylenode>
</stylenode>
</stylenode>
</map_styles>
</hook>
<hook NAME="AutomaticEdgeColor" COUNTER="4"/>
<edge STYLE="bezier" COLOR="#808080" WIDTH="1"/>
<node TEXT="Languages" LOCALIZED_STYLE_REF="defaultstyle.floating" POSITION="right" ID="ID_987657316" CREATED="1539509822363" MODIFIED="1539566191338" HGAP="-657" VSHIFT="-274">
<icon BUILTIN="wizard"/>
<hook NAME="FreeNode"/>
<font BOLD="true" ITALIC="true"/>
<node TEXT="Core" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1818400174" CREATED="1539530964681" MODIFIED="1539531041714" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="C" ID="ID_1145580181" CREATED="1539509877672" MODIFIED="1539530992573"/>
</node>
<node TEXT="Scripting" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1565957932" CREATED="1539530974984" MODIFIED="1539531046680" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Lua" ID="ID_856046975" CREATED="1539509891177" MODIFIED="1539530997197"/>
</node>
<node TEXT="Simulation" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_201746483" CREATED="1539531000537" MODIFIED="1539531048016" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Python" ID="ID_1341193772" CREATED="1539531009528" MODIFIED="1539531014981"/>
</node>
<node TEXT="SDN" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_23685208" CREATED="1539540696354" MODIFIED="1539540704186" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Frenetic" ID="ID_1146292884" CREATED="1539540708391" MODIFIED="1539540712565"/>
<node TEXT="Pyretic" ID="ID_1901986090" CREATED="1539540714118" MODIFIED="1539540718173"/>
<node TEXT="Kinetic" ID="ID_875235214" CREATED="1539540720350" MODIFIED="1539540725652"/>
</node>
</node>
<node TEXT="Theory" LOCALIZED_STYLE_REF="defaultstyle.floating" POSITION="right" ID="ID_609601182" CREATED="1539540759219" MODIFIED="1539637343405" HGAP="-341" VSHIFT="-266">
<icon BUILTIN="edit"/>
<hook NAME="FreeNode"/>
<font BOLD="true" ITALIC="true"/>
<node TEXT="A&amp;A" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1029021038" CREATED="1539540772887" MODIFIED="1539540842812" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Cellular Automata" ID="ID_333383246" CREATED="1539466677381" MODIFIED="1539467326416">
<icon BUILTIN="attach"/>
<hook NAME="FirstGroupNode"/>
</node>
<node TEXT="Agent-Based Simulation" ID="ID_30632382" CREATED="1539466688302" MODIFIED="1539467247817">
<icon BUILTIN="attach"/>
</node>
</node>
<node TEXT="Images" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_113259028" CREATED="1539540791303" MODIFIED="1539540842820" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Video Compression" ID="ID_992640191" CREATED="1539468926229" MODIFIED="1539469012679">
<icon BUILTIN="attach"/>
<hook NAME="FirstGroupNode"/>
</node>
<node TEXT="Image Feature Extraction" ID="ID_1582686599" CREATED="1539468938723" MODIFIED="1539469015967">
<icon BUILTIN="attach"/>
</node>
</node>
<node TEXT="SDN" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1545277509" CREATED="1539540802679" MODIFIED="1539540842820" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Verification" ID="ID_384787285" CREATED="1539541153750" MODIFIED="1539541546923" LINK="http://resonance.noise.gatech.edu/example.html">
<icon BUILTIN="attach"/>
</node>
</node>
<node TEXT="Graphs" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1955468515" CREATED="1539540821007" MODIFIED="1539540842821" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Network Science" ID="ID_1790368541" CREATED="1539544168341" MODIFIED="1539544208892" LINK="http://networksciencebook.com/">
<icon BUILTIN="attach"/>
</node>
</node>
<node TEXT="Automata" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1848400586" CREATED="1539628492358" MODIFIED="1539628513370" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="FSM Composition" ID="ID_1224117829" CREATED="1539628501117" MODIFIED="1539628526537">
<icon BUILTIN="attach"/>
</node>
</node>
</node>
<node TEXT="Tools" LOCALIZED_STYLE_REF="defaultstyle.floating" POSITION="right" ID="ID_1058084240" CREATED="1539529507077" MODIFIED="1539602230678" HGAP="598" VSHIFT="-67">
<icon BUILTIN="executable"/>
<hook NAME="FreeNode"/>
<font BOLD="true" ITALIC="true"/>
<node TEXT="RTS Engines" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_892937495" CREATED="1539530095467" MODIFIED="1539531110549" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Spring" ID="ID_1271938378" CREATED="1539479719179" MODIFIED="1539531244924">
<icon BUILTIN="revision"/>
<hook NAME="FirstGroupNode"/>
</node>
<node TEXT="Stratagus" ID="ID_523927183" CREATED="1539471405025" MODIFIED="1539531249452">
<icon BUILTIN="revision"/>
</node>
</node>
<node TEXT="Communications" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1218468407" CREATED="1539530161235" MODIFIED="1539531122061" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="SIPP" ID="ID_1561368170" CREATED="1539469509435" MODIFIED="1539529787690">
<hook NAME="FirstGroupNode"/>
</node>
<node TEXT="Seagull" ID="ID_1340478022" CREATED="1539469518453" MODIFIED="1539529787690"/>
<node TEXT="SIP Stack" ID="ID_266097576" CREATED="1539470862850" MODIFIED="1539531229213">
<icon BUILTIN="xmag"/>
</node>
<node TEXT="Diameter Stack" ID="ID_174883221" CREATED="1539470875164" MODIFIED="1539531234748">
<icon BUILTIN="xmag"/>
</node>
</node>
<node TEXT="Visualization" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1060975067" CREATED="1539531146923" MODIFIED="1539531180612" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Gully" ID="ID_113107029" CREATED="1539466437950" MODIFIED="1539529787688"/>
<node TEXT="GIS" ID="ID_1016269417" CREATED="1539470832162" MODIFIED="1539531267044">
<icon BUILTIN="xmag"/>
</node>
<node TEXT="SoNIA" ID="ID_1055278445" CREATED="1539548750602" MODIFIED="1539548765552">
<icon BUILTIN="revision"/>
</node>
</node>
<node TEXT="SDN" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_893829490" CREATED="1539531315155" MODIFIED="1539539122115" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Ryu" ID="ID_1603170511" CREATED="1539531320448" MODIFIED="1539539295281">
<icon BUILTIN="revision"/>
</node>
<node TEXT="Nox" ID="ID_329452461" CREATED="1539539259071" MODIFIED="1539539298115">
<icon BUILTIN="revision"/>
</node>
<node TEXT="Pox" ID="ID_268833257" CREATED="1539539263359" MODIFIED="1539539301363">
<icon BUILTIN="revision"/>
</node>
<node TEXT="MUL" ID="ID_920201520" CREATED="1539539267527" MODIFIED="1539539304867">
<icon BUILTIN="revision"/>
</node>
<node TEXT="Trema" ID="ID_1431106292" CREATED="1539539571984" MODIFIED="1539539589331">
<icon BUILTIN="revision"/>
</node>
<node TEXT="Open Daylight" ID="ID_1296380642" CREATED="1539539579384" MODIFIED="1539539587419">
<icon BUILTIN="revision"/>
</node>
<node TEXT="related/other" ID="ID_1683015708" CREATED="1539541721654" MODIFIED="1539541730691">
<node TEXT="Click" ID="ID_6867222" CREATED="1539541733390" MODIFIED="1539550478905">
<icon BUILTIN="revision"/>
</node>
<node TEXT="NetFPGA" ID="ID_719869891" CREATED="1539541739743" MODIFIED="1539550475705">
<icon BUILTIN="revision"/>
</node>
</node>
</node>
<node TEXT="Graphs" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_323151793" CREATED="1539539131055" MODIFIED="1539539156261" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="Gephi" ID="ID_1105822709" CREATED="1539466454029" MODIFIED="1539529828364"/>
<node TEXT="NetworkX" ID="ID_553167803" CREATED="1539539184231" MODIFIED="1539550469465" LINK="https://networkx.github.io/">
<icon BUILTIN="revision"/>
</node>
</node>
<node TEXT="Simulation" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1239645891" CREATED="1539548802092" MODIFIED="1539548843039" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="TRANSIMS" ID="ID_653395054" CREATED="1539548808036" MODIFIED="1539550446793" LINK="https://sourceforge.net/projects/transims/">
<icon BUILTIN="revision"/>
</node>
<node TEXT="MATSIM" ID="ID_584090139" CREATED="1539550421573" MODIFIED="1539550462777" LINK="https://www.matsim.org">
<icon BUILTIN="revision"/>
</node>
<node TEXT="GIS+ABM" ID="ID_1393491840" CREATED="1539553917597" MODIFIED="1539553979017" LINK="https://www.gisagents.org/">
<icon BUILTIN="revision"/>
</node>
</node>
<node TEXT="Parallel" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1746375755" CREATED="1539550564635" MODIFIED="1539550582528" STYLE="bubble">
<edge STYLE="bezier"/>
<node TEXT="CUDA" ID="ID_552358840" CREATED="1539550571709" MODIFIED="1539550577217"/>
</node>
</node>
<node TEXT="RTS Engines" LOCALIZED_STYLE_REF="styles.subsubtopic" POSITION="right" ID="ID_270030227" CREATED="1539529287710" MODIFIED="1539556542170" HGAP="-524" VSHIFT="9" STYLE="bubble">
<hook NAME="FreeNode"/>
<edge STYLE="hide_edge" COLOR="#7c7c00"/>
<arrowlink SHAPE="CUBIC_CURVE" COLOR="#000000" WIDTH="2" TRANSPARENCY="80" FONT_SIZE="9" FONT_FAMILY="SansSerif" DESTINATION="ID_1885745765" STARTINCLINATION="46;0;" ENDINCLINATION="96;0;" STARTARROW="NONE" ENDARROW="DEFAULT"/>
</node>
<node TEXT="Sample applications" LOCALIZED_STYLE_REF="styles.subtopic" POSITION="left" ID="ID_140010758" CREATED="1539466487757" MODIFIED="1539564997779" HGAP="64" VSHIFT="-10" STYLE="bubble">
<icon BUILTIN="video"/>
<edge COLOR="#808080"/>
<node TEXT="CA &amp; ABS overlays" ID="ID_499856843" CREATED="1539466514829" MODIFIED="1539556484393">
<node TEXT="CNS Functions Simulation" ID="ID_1983773101" CREATED="1539469388819" MODIFIED="1539556458208">
<icon BUILTIN="video"/>
<icon BUILTIN="bookmark"/>
</node>
<node TEXT="Order from Chaos" ID="ID_615404898" CREATED="1539469400812" MODIFIED="1539555930584">
<icon BUILTIN="video"/>
<icon BUILTIN="bookmark"/>
</node>
<node TEXT="Traffic Microsimulation" ID="ID_84231271" CREATED="1539470797034" MODIFIED="1539555946332">
<icon BUILTIN="video"/>
</node>
<node TEXT="Cellular Automata Interaction" ID="ID_769778685" CREATED="1539558344931" MODIFIED="1539558561136">
<icon BUILTIN="video"/>
<icon BUILTIN="bookmark"/>
<icon BUILTIN="help"/>
</node>
</node>
<node TEXT="SIP/Diameter Server" ID="ID_1826293617" CREATED="1539466654189" MODIFIED="1539556273363">
<icon BUILTIN="video"/>
</node>
<node TEXT="RTS Games" ID="ID_1885745765" CREATED="1539469158541" MODIFIED="1539556273362">
<icon BUILTIN="video"/>
</node>
<node TEXT="Multi-Source Object Tracking" ID="ID_1304989791" CREATED="1539468883003" MODIFIED="1539469563336">
<icon BUILTIN="video"/>
</node>
<node TEXT="SDN Controller" ID="ID_5528235" CREATED="1539529190561" MODIFIED="1539556273362">
<icon BUILTIN="video"/>
</node>
<node TEXT="Reconfigurable IP Router" LOCALIZED_STYLE_REF="default" ID="ID_943823787" CREATED="1539541811049" MODIFIED="1539595422812">
<icon BUILTIN="video"/>
</node>
<node TEXT="IoT / Sensor Network Controller" ID="ID_249411539" CREATED="1539558237179" MODIFIED="1539567747476">
<icon BUILTIN="video"/>
<icon BUILTIN="xmag"/>
</node>
<node TEXT="Real-Time System Control" ID="ID_1966130297" CREATED="1539558257387" MODIFIED="1539558614695">
<icon BUILTIN="video"/>
<icon BUILTIN="bookmark"/>
<icon BUILTIN="xmag"/>
</node>
</node>
<node TEXT="Roadmap" LOCALIZED_STYLE_REF="styles.subtopic" POSITION="right" ID="ID_376961700" CREATED="1539564139103" MODIFIED="1539600167709" HGAP="35" VSHIFT="-158" STYLE="bubble">
<icon BUILTIN="launch"/>
<edge COLOR="#808080"/>
<node TEXT="Step 1" LOCALIZED_STYLE_REF="styles.important" ID="ID_618600153" CREATED="1539565874630" MODIFIED="1539566240698" STYLE="bubble">
<edge COLOR="#808080"/>
<node TEXT="Applications" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_917363" CREATED="1539564573207" MODIFIED="1539565922465" STYLE="bubble">
<edge COLOR="#808080"/>
<node TEXT="TIME" ID="ID_706259130" CREATED="1539564240103" MODIFIED="1539602212799">
<icon BUILTIN="full-1"/>
<cloud COLOR="#00ffff" SHAPE="ARC"/>
</node>
<node TEXT="TCPP" ID="ID_1377597942" CREATED="1539564298727" MODIFIED="1539566342538">
<icon BUILTIN="full-4"/>
</node>
<node TEXT="UDPP" ID="ID_221231048" CREATED="1539564413855" MODIFIED="1539566339058">
<icon BUILTIN="full-3"/>
</node>
</node>
<node TEXT="Samples" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1518961667" CREATED="1539565931951" MODIFIED="1539565944561" STYLE="bubble">
<edge COLOR="#808080"/>
<node TEXT="Cellular Agents" ID="ID_1607747196" CREATED="1539566017142" MODIFIED="1539602224431">
<icon BUILTIN="full-2"/>
<icon BUILTIN="help"/>
<cloud COLOR="#00ffff" SHAPE="ARC"/>
</node>
</node>
</node>
<node TEXT="Step 2" LOCALIZED_STYLE_REF="styles.important" ID="ID_1391430144" CREATED="1539566227215" MODIFIED="1539566293677" STYLE="bubble">
<edge COLOR="#808080"/>
<node TEXT="Samples" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_224218727" CREATED="1539566253693" MODIFIED="1539566293677" STYLE="bubble" VSHIFT="20">
<edge COLOR="#808080"/>
<node TEXT="SIP Server" ID="ID_864675425" CREATED="1539566273109" MODIFIED="1539566351962">
<icon BUILTIN="full-1"/>
</node>
</node>
</node>
</node>
<node TEXT="SDN" LOCALIZED_STYLE_REF="styles.subsubtopic" POSITION="right" ID="ID_154521788" CREATED="1539539363812" MODIFIED="1539556536873" HGAP="-460" VSHIFT="63" STYLE="bubble">
<hook NAME="FreeNode"/>
<edge STYLE="hide_edge" COLOR="#ff0000"/>
<arrowlink SHAPE="CUBIC_CURVE" COLOR="#000000" WIDTH="2" TRANSPARENCY="80" FONT_SIZE="9" FONT_FAMILY="SansSerif" DESTINATION="ID_5528235" STARTINCLINATION="100;0;" ENDINCLINATION="100;0;" STARTARROW="NONE" ENDARROW="DEFAULT"/>
<arrowlink SHAPE="CUBIC_CURVE" COLOR="#000000" WIDTH="2" TRANSPARENCY="80" FONT_SIZE="9" FONT_FAMILY="SansSerif" DESTINATION="ID_943823787" STARTINCLINATION="61;0;" ENDINCLINATION="61;0;" STARTARROW="NONE" ENDARROW="DEFAULT"/>
</node>
<node TEXT="Protocols" LOCALIZED_STYLE_REF="defaultstyle.floating" POSITION="right" ID="ID_1561170580" CREATED="1539595466084" MODIFIED="1539637355589" HGAP="234" VSHIFT="170">
<icon BUILTIN="Mail"/>
<hook NAME="FreeNode"/>
<node TEXT="IoT" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1887133209" CREATED="1539599706308" MODIFIED="1539599801266" STYLE="bubble">
<edge STYLE="bezier" COLOR="#808080"/>
<node TEXT="MQTT" ID="ID_1217300124" CREATED="1539595537439" MODIFIED="1539595541452">
<node TEXT="SMQTT" ID="ID_1488207811" CREATED="1539599902392" MODIFIED="1539599907976"/>
</node>
<node TEXT="CoAP" ID="ID_100660733" CREATED="1539599829868" MODIFIED="1539599832992"/>
<node TEXT="AMQP" ID="ID_283054608" CREATED="1539599884188" MODIFIED="1539599887696"/>
<node TEXT="DDS" ID="ID_1773050178" CREATED="1539600124539" MODIFIED="1539600127344"/>
</node>
<node TEXT="SDN" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_818959239" CREATED="1539599714701" MODIFIED="1539599801264" STYLE="bubble">
<edge STYLE="bezier" COLOR="#808080"/>
<node TEXT="OpenFlow" ID="ID_1552787413" CREATED="1539595501247" MODIFIED="1539595528252"/>
<node TEXT="NETCONF" ID="ID_1434419438" CREATED="1539600025508" MODIFIED="1539600029616"/>
<node TEXT="SNMP" ID="ID_1744640597" CREATED="1539600032428" MODIFIED="1539600038928"/>
</node>
<node TEXT="Communications" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_809564213" CREATED="1539599731645" MODIFIED="1539599801266" STYLE="bubble">
<edge STYLE="bezier" COLOR="#808080"/>
<node TEXT="SIP" ID="ID_955519259" CREATED="1539595487743" MODIFIED="1539595491564"/>
<node TEXT="Diameter" ID="ID_1309662556" CREATED="1539595493175" MODIFIED="1539595499196"/>
<node TEXT="XMPP" ID="ID_740780568" CREATED="1539599820268" MODIFIED="1539599824504"/>
</node>
<node TEXT="*" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_1553305633" CREATED="1539600216624" MODIFIED="1539600230126" STYLE="bubble">
<edge STYLE="bezier" COLOR="#808080"/>
<node TEXT="HTTP / REST" ID="ID_34040447" CREATED="1539600154555" MODIFIED="1539600185536">
<edge STYLE="bezier"/>
</node>
</node>
</node>
<node TEXT="Courses" LOCALIZED_STYLE_REF="defaultstyle.floating" POSITION="right" ID="ID_1843258087" CREATED="1539527534310" MODIFIED="1539566205825" HGAP="-669" VSHIFT="237">
<icon BUILTIN="pencil"/>
<hook NAME="FreeNode"/>
<font BOLD="true" ITALIC="true"/>
<node TEXT="SDN" LOCALIZED_STYLE_REF="styles.subsubtopic" ID="ID_695318409" CREATED="1539527556590" MODIFIED="1539542918468" STYLE="bubble" VSHIFT="7">
<edge STYLE="bezier"/>
<node TEXT="Coursera: Cloud Networking" ID="ID_167874650" CREATED="1539527576097" MODIFIED="1539527667920" LINK="https:/www.coursera.org/learn/cloud-networking"/>
<node TEXT="Coursera: SDN 2013" ID="ID_304014782" CREATED="1539527718226" MODIFIED="1539527953711" LINK="https://www.youtube.com/playlist?list=PLpherdrLyny8YN4M24iRJBMCXkLcGbmhY"/>
<node TEXT="Coursera: SDN 2014" ID="ID_1715478370" CREATED="1539527978913" MODIFIED="1539528004512" LINK="https://www.youtube.com/watch?v=I-XdDffLMqc&amp;list=PLpherdrLyny-4Y6jXKvi0Ia9jJAk3M_Bs"/>
<node TEXT="Udacity: Computer Networking" ID="ID_969034982" CREATED="1539528408653" MODIFIED="1539528440479" LINK="https://classroom.udacity.com/courses/ud436"/>
</node>
</node>
</node>
</map>
