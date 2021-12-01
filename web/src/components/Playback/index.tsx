import { PlaybackState, PlaybackStatus } from "../../api/euphonium/models";
import {
  eventSource,
  getPlaybackState,
  updateVolume,
} from "../../api/euphonium/api";
import Icon from "../Icon";
import { useEffect, useState } from "preact/hooks";
import useIsMobile from "../../utils/isMobile.hook";
import Equalizer from "../Equalizer";

const PlaybackBar = () => {
  const [playbackState, setPlaybackState] = useState<PlaybackState>();

  useEffect(() => {
    getPlaybackState().then((e) => setPlaybackState(e));
  }, []);

  useEffect(() => {
    eventSource.addEventListener("playback", ({ data }: any) => {
      console.log(data);
      setPlaybackState(JSON.parse(data));
    });
  }, []);

  const [eqOpen, setEqOpen] = useState<boolean>(false);

  const volUpdated = (volume: number) => {
    updateVolume(Math.round((volume / 15) * 100));
  };

  const isMobile = useIsMobile();

  return (
    <div class="flex flex-row bg-grey-light md:bg-gray-700 h-13 mr-3 ml-3 rounded-t-xl items-center shadow-xl flex-grow">
      <img
        class="rounded-xl h-20 w-20 shadow-xl bg-white -mt-8 ml-2 mb-2"
        src={playbackState?.song.icon}
      ></img>
      <div class="flex flex-col justify-center ml-4">
        <div>{playbackState?.song.songName}</div>
        <div class="text-gray-400 text-xs">
          {playbackState?.song.artistName} • {playbackState?.song.albumName} • playback
          from {playbackState?.song.sourceName}
        </div>
      </div>
      <div></div>
      {!isMobile ? (
        <div class="mr-3 text-xl text-gray-400 ml-auto flex flex-row">
          <div class="relative flex justify-center" onClick={(e) => setEqOpen(!eqOpen)}>
            {eqOpen && playbackState?.eq ? (
              <div class="absolute bottom-[50px] shadow-xl text-center rounded-xl w-[170px] p-4 z-index-2 bg-gray-700 mb-1">
                <Equalizer eq={playbackState!!.eq} />
              </div>
            ) : null}
            <Icon name="eq" />
          </div>
          <Icon name="vol-up" />
          <input
            class="w-20"
            type="range"
            id="volume"
            name="volume"
            value={((playbackState?.volume || 0) / 100) * 15}
            onInput={(e: any) => volUpdated(e.target.value)}
            min="0"
            max="15"
          />
          <Icon name={playbackState?.status == PlaybackStatus.Playing ?  "pause" : "play"} />
        </div>
      ) : null}
    </div>
  );
};

export default () => {
  return (
    <div class="flex flex-grow bottom-0 fixed right-0 left-0 md:left-[220px]">
      <PlaybackBar />
    </div>
  );
};