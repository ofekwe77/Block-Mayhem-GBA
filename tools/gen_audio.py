"""Generates the original BLOCK MAYHEM soundtrack (XM modules) and sound effects
(WAV) into audio/. Run: python3 tools/gen_audio.py
"""
import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "audio"))

import composer  # noqa: E402
import sfx  # noqa: E402
import songs  # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
AUDIO = os.path.join(ROOT, "audio")


def main():
    os.makedirs(AUDIO, exist_ok=True)
    for name, song in songs.SONGS.items():
        composer.build_song(song, os.path.join(AUDIO, name + ".xm"))
    for name in songs.HOT:
        composer.build_song(songs.SONGS[name], os.path.join(AUDIO, name + "_hot.xm"), hot=True)
    for name, fn in sfx.ALL.items():
        fn().write(os.path.join(AUDIO, name + ".wav"))
    print("audio generated: %d songs, %d hot variants, %d sound effects"
          % (len(songs.SONGS), len(songs.HOT), len(sfx.ALL)))


if __name__ == "__main__":
    main()
