<?xml version="1.0" encoding="UTF-8"?>

<project>
    <version>1</version>
    <url>/home/jon/Videos/sssamples.ove</url>

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
        <frequency>48000</frequency>
        <layout>0</layout>
      </audio>
      <audio id="2">
        <channels>5</channels>
        <frequency>96000</frequency>
        <layout>0</layout>
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
    <sequence id="1" folder="0">
      <workarea using="true" enabled="true" in="0" out="0"/>
      <name>Test Sequence</name>
      <width>1920</width>
      <height>1080</height>
      <framerate>25.0000000000</framerate>
      <frequency>48000</frequency>
      <layout>2</layout>
      <clip id="0" enabled="true" link_id="0" in="0" out="400">
        <name>clipcanvas_14348_Avid_DNxHD.mov</name>
        <clipin>0</clipin> <!-- ? -->
        <track>-1</track>
        <opening>-1</opening> <!-- ? -->

        <effect enabled="true" name="transform">
          <field>
            <name>posx</name>
            <value>720</value>
          </field>
          <field>
            <name>posy</name>
            <value>540</value>
          </field>
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

