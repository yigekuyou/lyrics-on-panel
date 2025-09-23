import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import QtQuick.Window 2.15
import io.github.lyric 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.private.mpris as Mprisplasma
import org.kde.kirigami 2.15 as Kirigami

/**
Below are some documents that I found useful when writing this widget.

https://specifications.freedesktop.org/mpris-spec/latest/Player_Interface.html

https://app.readthedocs.org/projects/mpris2/downloads/pdf/latest/
*/


PlasmoidItem {
    id: root
    Mprisplasma.Mpris2Model {
	    id: mpris2Model
	    onCurrentPlayerChanged: {
		    // 当播放器改变时，调用 reset() 并获取新歌词
		    reset();
		    // 确保播放器有效
		    if (mpris2Model.currentPlayer) {
			    lyricSource.findAndGetAsText(mpris2Model.currentPlayer.identity);
		    }
	}
    }

    // Seems obsolete by KDE Plasma 6.
    Mprisplasma.MultiplexerModel {
        id: multiplexerModel
    }

    Mpris {
	    id: lyricSource
    }
    width: 0;
    height: config_lyricTextSize;

    // Need to set it full representation. Otherwise it will only display the applet icon declared in the metadata.json file on the panel.
    preferredRepresentation: fullRepresentation
    Layout.preferredWidth: Plasmoid.status == PlasmaCore.Types.HiddenStatus ? 0 : config_preferedWidgetWidth+5 * config_mediaControllItemSize + 4 * config_mediaControllSpacing;
    Layout.preferredHeight: config_lyricTextSize;

    /**
        Set the background of this widget to be 'configurable' transparent or non transparent.
        https://develop.kde.org/docs/plasma/widget/properties/#x-plasma-api-x-plasma-mainscript
    */
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground | PlasmaCore.Types.ConfigurableBackground

    // Should ask uiYzzi if problem occurs.
    Plasmoid.status: mpris2Model.currentPlayer?.canControl || !config_hideItemWhenNoControlChecked ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.HiddenStatus;
    TextMetrics {
	    id: lyricMetrics
	    // 绑定到 Label 的 font 和 text
	    font.pixelSize: config_lyricTextSize
    }
    ListView {
	    id: lyricListView
	    interactive: false
	    height: parent.height
	    // 将 ListView 的右边界与 iconsContainer 的左边界对齐
	    anchors.right: iconsContainer.left
	    anchors.left: parent.left
	    anchors.verticalCenter: parent.verticalCenter
	    anchors.verticalCenterOffset: config_lyricTextVerticalOffset
	    clip: true
	    flickableDirection: Flickable.AutoFlickDirection
	    orientation: ListView.Horizontal // 设置为水平滚动
	    cacheBuffer:lyricsWTimes.count
	    model: lyricsWTimes
	    spacing:config_preferedWidgetWidth
	    // 歌词条目的委托
	    delegate: PlasmaComponents.Label {
		Layout.preferredWidth: implicitWidth
		    text: model.lyric
		horizontalAlignment: Text.AlignRight
		    color: config_lyricTextColor
		    font.pixelSize: config_lyricTextSize
		    font.bold: config_lyricTextBold
		    font.italic: config_lyricTextItalic
		    anchors.verticalCenterOffset: config_lyricTextVerticalOffset
	     }
	     onCurrentIndexChanged: {
		     if(lyricsWTimes.count > 0 && currentLyricIndex >= 0 ){
		     lyricListView.positionViewAtIndex(currentLyricIndex, ListView.Right)
		     lyricScrollAnimation.stop();
			lyricMetrics.text = lyricsWTimes.get(currentLyricIndex).lyric;
		     if (lyricMetrics.advanceWidth > width) {
				if (currentLyricIndex + 2 < lyricsWTimes.count) {
				lyricScrollAnimation.duration = (lyricsWTimes.get(currentLyricIndex+1).time - mprisCurrentPlayingSongTimeMS)/1000 ; //这是从计算器里验证的ms
				}
				if( (lyricsWTimes.get(currentLyricIndex+1).time - mprisCurrentPlayingSongTimeMS) <0)
			     {
				     lyricScrollAnimation.duration=lyricMetrics.advanceWidth * 200
			     }
			     lyricScrollAnimation.from = contentX - width ;
			     lyricScrollAnimation.to = contentX - width + lyricMetrics.advanceWidth;
			     lyricScrollAnimation.start();
		     }
		     }
	     }
    }
    Binding {
	    target: lyricListView
	    property: "currentIndex"
	    value: currentLyricIndex
    }
    PropertyAnimation {
	    id: lyricScrollAnimation
	    target: lyricListView
	    property: "contentX"
	    easing.type: Easing.Linear
    }
    Item {
	    id: iconsContainer
	    anchors.left: 6 * (config_mediaControllItemSize + config_mediaControllSpacing)
	    anchors.right: parent.right
	    anchors.verticalCenter: parent.verticalCenter
	    width: 5 * config_mediaControllItemSize + 4 * config_mediaControllSpacing
	    height: config_mediaControllItemSize
	    anchors.verticalCenterOffset: config_mediaControllItemVerticalOffset

	    // Previous Button
	    Kirigami.Icon {
		    source: "media-skip-backward-symbolic"
		    width: config_mediaControllItemSize
		    height: config_mediaControllItemSize
		    anchors.left: parent.left
		    anchors.verticalCenter: parent.verticalCenter

		    MouseArea {
			    anchors.fill: parent
			    onClicked: {
				    previous();
			    }
		    }
	    }

	    // Play/Pause Button
	    Kirigami.Icon {
		    source: (playbackStatus == 2) ? "media-playback-pause-symbolic" : "media-playback-start-symbolic"
		    width: config_mediaControllItemSize
		    height: config_mediaControllItemSize
		    anchors.left: parent.left
		    anchors.leftMargin: config_mediaControllItemSize + config_mediaControllSpacing
		    anchors.verticalCenter: parent.verticalCenter

		    MouseArea {
			    anchors.fill: parent
			    onClicked: {
				    if (playbackStatus == 2) {
					    pause();
				    } else {
					    play();
				    }
			    }
		    }
	    }

	    // Next Button
	    Kirigami.Icon {
		    source: "media-skip-forward-symbolic"
		    width: config_mediaControllItemSize
		    height: config_mediaControllItemSize
		    anchors.left: parent.left
		    anchors.leftMargin: 2 * (config_mediaControllItemSize + config_mediaControllSpacing)
		    anchors.verticalCenter: parent.verticalCenter

		    MouseArea {
			    anchors.fill: parent
			    onClicked: {
				    next();
			    }
		    }
	    }

	    // Like/Unlike Button
	    Kirigami.Icon {
		    source: liked ? "rating-active-symbolic" : "rating-symbolic"
		    width: config_mediaControllItemSize
		    height: config_mediaControllItemSize
		    anchors.left: parent.left
		    anchors.leftMargin: 3 * (config_mediaControllItemSize + config_mediaControllSpacing)
		    anchors.verticalCenter: parent.verticalCenter

		    MouseArea {
			    anchors.fill: parent
			    onClicked: {
				    if (liked) {
					    liked = false;
				    } else {
					    liked = true;
				    }
			    }
		    }
	    }

	    // Media Player Icon
	    Kirigami.Icon {
		    source: "applications-multimedia-symbolic"
		    width: config_mediaControllItemSize
		    height: config_mediaControllItemSize
		    anchors.left: parent.left
		    anchors.leftMargin: 4 * (config_mediaControllItemSize + config_mediaControllSpacing)
		    anchors.verticalCenter: parent.verticalCenter
		    MouseArea {
			    anchors.fill: parent
			    onClicked: {
			    }
		    }
	    }
    }

    // UI-Resources related configurations
    property bool liked: false;


    // Applet UI behavior configuration
    property int config_lyricTextSize: Plasmoid.configuration.lyricTextSize;
    property string config_lyricTextColor: Plasmoid.configuration.lyricTextColor;
    property bool config_lyricTextBold: Plasmoid.configuration.lyricTextBold;
    property bool config_lyricTextItalic: Plasmoid.configuration.lyricTextItalic;
    property int config_lyricTextVerticalOffset: Plasmoid.configuration.lyricTextVerticalOffset

    property int config_mediaControllSpacing: Plasmoid.configuration.mediaControllSpacing
    property int config_mediaControllItemSize: Plasmoid.configuration.mediaControllItemSize
    property int config_mediaControllItemVerticalOffset: Plasmoid.configuration.mediaControllItemVerticalOffset;

    property int config_whiteMediaControlIconsChecked: Plasmoid.configuration.whiteMediaControlIconsChecked;
    property int config_preferedWidgetWidth: Plasmoid.configuration.preferedWidgetWidth;
    property bool config_hideItemWhenNoControlChecked: Plasmoid.configuration.hideItemWhenNoControlChecked;


    function handleAsTextChanged(){
	    reset()
	    if(Plasmoid.status){
		    if (lyricSource.asText===""){
			  lyricsWTimes.append({time: 0, lyric: currentMediaTitle})
		}
		    parseLyric(lyricSource.asText)
	}else {
		lyricsWTimes.append({time: 0, lyric: "no play"})
	}
}
    Connections {
	    target: lyricSource
	    function onAsTextChanged() {
		    // 此处只处理已获取的歌词，不再发起新的获取请求
		    handleAsTextChanged()
	    }
    }
    Timer {
        id: positionTimer
        interval: 1
        running: true
        repeat: true
        onTriggered: {
            mpris2Model.currentPlayer.updatePosition();
        }
    }
    Timer {
	    id: lyricDisplayTimer
	    interval: 1
	    running: false
	    repeat: true
	    onTriggered: {
		    // If the current playing media source in mpris2 datasource doesn't match the expected media source, then no lyric will be displayed
		    for (let i = 0; i < lyricsWTimes.count; i++) {
			    if (lyricsWTimes.get(i).time >= mprisCurrentPlayingSongTimeMS) {
				currentLyricIndex = i > 0 ? i - 1 : 0;
				break
			    }else{
				    if (!i){continue}
				    if(i>currentLyricIndex){
					currentLyricIndex =i
				}
			}
		}
	    }
    }
    // Global constant
    // Current Media Title (Song's name), default is empty string
    property string currentMediaTitle: mpris2Model.currentPlayer?.track ?? ""

    // Current Media Artists (Song's artist), default is empty string
    property string currentMediaArtists: mpris2Model.currentPlayer?.artist ?? ""

    // Current Media Album (Song's album), default is empty string
    property string currentMediaAlbum: mpris2Model.currentPlayer?.album ?? ""

    // Current Media Playback Status (Song's playback status), default is 0
    property int playbackStatus: mpris2Model.currentPlayer?.playbackStatus ?? -1

    // Retrieve if the current media is playing (Unused)
    property bool isPlaying: root.playbackStatus === Mprisplasma.PlaybackStatus.Playing

    // Retrieve the identity of current music/media player
    // YesPlayMusic Spotify lx-music-desktop xxx
    property string mpris2CurrentPlayerIdentity: mpris2Model.currentPlayer?.identity ?? ""

    // Retrieve the current media position (in microseconds)
    property int position: mpris2Model.currentPlayer?.position ?? 0

    property string prevNonEmptyLyric: ""

    /**
        A list of dictionaries. Each dictionary contains a timestamp and the corresponding lyric. Below is an example

        [
            {timestamp: 1, lyric: "Hello"},
            {timestamp: 2, lyric: "World"},
            {timestamp: 3, lyric: "!"}
        ]
    */
    ListModel {
        id: lyricsWTimes
    }

    // Other Media Player's mpris2 data
    property int mprisCurrentPlayingSongTimeMS: {
        if (position == 0) {
            return -1;
        } else {
            return position;
        }
    }

    // Just the index of the LyricWTimes lists. Retrieve the element from the list using the index. The retrieved element contains a timestamp and the corresponding lyric.
    property int currentLyricIndex: 0

    property string previousMediaTitle: ""

    property string previousMediaArtists: ""

    property string mpris2PreviousPlayerIdentity: ""

    property string prevExpectedPlayerIdentity: "";


    /**
        Parse the lyric file and convert it to a list of dictionaries. Each dictionary contains a timestamp and the corresponding lyric.
        The format of the lyric file is as follows:
        [00:34.33] 妳說這一句 很有夏天的感覺
        [00:41.06] 手中的鉛筆 在紙上來來回回
        [00:47.45] 我用幾行字形容妳是我的誰
        [00:54.19] 秋刀魚 的滋味 貓跟妳都想瞭解
    */
    function parseLyric(lrcFile) {
	    // console.log(lrcFile)
	    var lrcList = lrcFile.split("\n");
	    for (var i = 0; i < lrcList.length; i++) {
            // 找到第一个 ']' 的位置
		    var firstBracketIndex = lrcList[i].indexOf("]");
            // 确保找到了 ']' 并且它后面有内容
            if (firstBracketIndex !== -1 && lrcList[i].length > firstBracketIndex + 1) {
			    // 提取时间戳部分
			    var timeString = lrcList[i].substring(1, firstBracketIndex); // 从位置 1 开始，排除 '['
			    // 提取歌词部分
                var lyricPerRow = lrcList[i].substring(firstBracketIndex + 1).trim();
                var timestamp = parseTime(timeString);
                lyricsWTimes.append({time: timestamp, lyric: lyricPerRow});
            }
	    }
	    if (Plasmoid.status){
		    lyricDisplayTimer.start()
	}
    }
    function log() {
        console.log("currentMediaArtists: ", currentMediaArtists);
        console.log("previousMediaArtists: ", previousMediaArtists);
        console.log("currentMediaTitle: ", currentMediaTitle);
        console.log("previousMediaTitle: ", previousMediaTitle);
        console.log("Mpris2 Model: ", JSON.stringify(mpris2Model))
        console.log("Current Player Identity: ", mpris2CurrentPlayerIdentity);
        console.log(mpris2Model);
        console.log(mpris2Model.toString());
    }

    function parseTime(timeString) {
        var parts = timeString.split(":");
        var minutes = parseInt(parts[0], 10);
        var seconds = parseFloat(parts[1]);
        var parsedMicrosecond = (minutes * 60 + seconds) * 1000000
        return parsedMicrosecond;
    }

    function previous() {
           mpris2Model.currentPlayer.Previous();
    }

    function play() {
           mpris2Model.currentPlayer.Play();
    }

    function pause() {
            mpris2Model.currentPlayer.Pause();
    }

    function next() {
           mpris2Model.currentPlayer.Next();

    }


    /**
        1. Stop the compatible mode timer and yesplaymusic timer.
        2. Set the previous media title and artists to the current media title and artists.
        3. Set the previous player name to the current player name.
        4. Set the previous expected player Identity to the current expected player Identity
        5. Clear the lyricsWTimes list.
        6. Clear the previous non empty lyric.
        7. Clear the previous lrc id.
        8. Set the fallback mode to false, meaning that first query the lrclibAPI with precise matching, if failed, then go to the fallback mode.
        9. Set compatibleLRCFound to false, meaning that we haven't found the lyric yet(From LrcLib for compatible(global)/spotify mode).
        10. Set isYPMLyricFound to false, meaning that we haven't found the lyric yet(From YPM, YPM mode only).
    */
    function reset() {
        previousMediaTitle = currentMediaTitle;
        previousMediaArtists = currentMediaArtists;
        mpris2PreviousPlayerIdentity = mpris2CurrentPlayerIdentity;
        prevExpectedPlayerIdentity = "";
        lyricsWTimes.clear();

	lyricScrollAnimation.stop();
	prevNonEmptyLyric= ""
	lyricListView.positionViewAtBeginning()

    }
}
