import Qt 4.7
import MeeGo.Media 0.1
import QtMultimediaKit 1.1

Item {
    id: container
    property string soundSource: ""
    property bool repeat: false

    function stopSound()
    {
        audio.stop();
    }

    function playSound()
    {
        audio.play();
    }

    signal endOfMedia

    Audio {
        id: audio
        source: container.soundSource
        onError: {
            console.log("error: " + error + " - description: " + errorString);
        }

        onStopped: {
            if(container.repeat && status == Audio.EndOfMedia) {
                audio.play();
            }
        }

        onStatusChanged: {
            if (status == Audio.EndOfMedia) {
                container.endOfMedia();
            }
        }
    }
}
