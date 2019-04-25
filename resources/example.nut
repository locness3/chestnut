<?xml version="1.0" encoding="UTF-8"?>

<project>
    <version>1</version>

    <folders>
        <folder id="1" parent="0">
          <name>testy</name>
        </folder>
    </folders>

  <media>
    <footage folder="0" id="1" using_inout="false" in="0" out="0">
      <name>clipcanvas_14348_Avid_DNxHD.mov</name>
      <url>/home/jon/Videos/clipcanvas_14348_Avid_DNxHD.mov</url>
      <duration>8000000</duration>
      <video id="0" infinite="false">
        <width>1920</width>
        <height>1080</height>
        <framerate>25.0000000000</framerate>
      </video>
      <audio id="1">
        <channels>2</channels>
        <layout>0</layout>
        <frequency>48000</frequency>
      </audio>
      <audio id="2">
        <channels>5</channels>
        <layout>0</layout>
        <frequency>96000</frequency>
      </audio>

      <marker>
        <name>hello,world</name>
        <frame>70</frame>
        <comment>spam</comment>
        <duration>0</duration>
        <color>4294967295</color>
      </marker>
    </footage>
  </media>

  <sequences>
    <sequence id="1" folder="0" open="false">
      <workarea using="true" enabled="true" in="0" out="0"/>
      <name>Test Sequence</name>
      <width>1920</width>
      <height>1080</height>
      <framerate>25.0000000000</framerate>
      <frequency>48000</frequency>
      <layout>2</layout>
      <clip source="1" id="0">        
        <timelineinfo>
            <name>clipcanvas_14348_Avid_DNxHD</name>
            <clipin>0</clipin>
            <enabled>true</enabled>
            <in>0</in>
            <out>542</out>
            <track>0</track>
            <color>4286611648</color>
            <autoscale>false</autoscale>
            <speed>1</speed>
            <maintainpitch>1</maintainpitch>
            <reverse>0</reverse>
            <stream>1</stream>
        </timelineinfo>
        
        <opening_transition>
            <name>crossdissolve</name>
            <length>100</length>            
        </opening_transition>     
        <closing_transition>
            <name></name>
            <length>-1</length>
        </closing_transition>   
        
        <links/>

        <effect enabled="true" name="transform">
            <row>
                <name>Position</name>
                <field>
                    <name>posx</name>
                    <value>720</value>
                </field>
                <field>
                    <name>posy</name>
                    <value>540</value>
                </field>
            </row>
        </effect>
      </clip>

      <marker>
        <name>123</name>
        <frame>255</frame>
        <comment>eggs</comment>
        <duration>0</duration>
        <color>4294967295</color>
      </marker>

      <!-- etc,etc,etc -->
    </sequence>
  </sequences>
</project>

