# Stapik Media

A desktop media-tracking application for Linux, written in C++20 using GTK4/gtkmm. Styled after the retro old-school aesthetic.

![Screenshot](screenshots/screenshot_1.png)

## Features

- **Category sidebar** - browse your entries by type: Movies, Series, Cartoons, Books, Albums, Games.
- **Media entries** - add, edit and delete entries, with fields tailored to each category (director/genre for movies and series, author/genre/audiobook for books, performer/publisher/genre for albums, studio/publisher/platform for games).
- **Release date with adjustable precision** - store a full date, a month and year, or just a year, depending on how much you actually know about older titles.
- **Consumption date** - track the month and year you watched/read/listened/played each entry, defaulting to the current month and year when adding a new one
- **Filtering** - narrow the list down by consumption year, and optionally by month within that year.
- **Pagination** - entries are shown 20 per page.
- **Undo / Redo** - add, edit and delete are undoable from the Edit menu (Ctrl+Z / Ctrl+Shift+Z).
- **Cloud sync** - background synchronization with a self-hosted Stapik Cloud server, with automatic conflict resolution based on timestamps and a status indicator in the window.
- **Yearly archives** - only the current year is kept in the main cloud document; older years are archived separately and downloaded on demand.
- **Multilingual UI** - Polish, English and German interface with instant switching.
- **Auto-save** - data saved locally after every change, with atomic writes and rotating backups (at most one every five minutes) that can be restored from the File menu.
- **Remembers your setup** - window size, last selected category, language and theme.
- **Retro aesthetic** — classic look with raised buttons, blue navigation bar and grey cells.
- **Five themes** - Classic, Classic Pink, Modern, Dark and Neoclassic, switchable live from the menu.

## Dependencies

- `gtkmm-4.0`
- `libcurl`
- [`stapik-common`](https://github.com/stapik/stapik-common) (fetched automatically via CMake FetchContent)
- `nlohmann/json` (fetched automatically via CMake FetchContent, transitively provided by `stapik-common`)

On Ubuntu/Debian:
```bash
sudo apt install libgtkmm-4.0-dev libcurl4-openssl-dev
```

Building a `.deb` package additionally requires `dpkg-dev` (used to auto-detect runtime dependencies):
```bash
sudo apt install dpkg-dev
```

Running the unit tests needs nothing extra: GoogleTest is fetched automatically when the tests are enabled.

## Building

```bash
git clone https://github.com/Stapik-Group/stapik-media
cd stapik-media
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release
```

### Continuous integration

GitHub Actions builds and tests every branch except `develop` and every pull request to `main`. Pushing a tag `vX.Y.Z` that matches the project version and lies on `main` attaches the `.deb` package and the Windows zip to a GitHub release.

### Running the tests

```bash
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DSTAPIK_MEDIA_BUILD_TESTS=ON
cmake --build cmake-build-debug --target stapikmedia_tests
ctest --test-dir cmake-build-debug --output-on-failure
```

## Installation

### Option 1 — Download prebuilt `.deb` (recommended)

Download the latest `.deb` package from the [Releases page](https://github.com/Stapik-Group/stapik-media/releases), then install it:

```bash
sudo dpkg -i stapikmedia_*.deb
sudo apt install -f   # resolves any missing runtime dependencies
```

### Option 2 — build `.deb` from source

If you'd rather build the package yourself:

```bash
cd cmake-build-release
cpack -G DEB
sudo dpkg -i stapikmedia_*.deb
sudo apt install -f
```

Either option installs the app to `/opt/stapikmedia/` (binary and resources), with a launcher at `/usr/bin/stapikmedia`, a `pl.stapik.media.desktop` entry and an icon under `/usr/share`, so it appears in the desktop environment's application menu.

### Option 3 — per-user install (no sudo required)

```bash
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="$HOME/.local/opt/stapikmedia" \
      -DSTAPIK_APP_SYSTEM_PREFIX="$HOME/.local"
cmake --build cmake-build-release
cmake --install cmake-build-release
```

Installs the binary and resources to `~/.local/opt/stapikmedia/`, with a launcher at `~/.local/bin/stapikmedia`. Make sure `~/.local/bin` is in your `PATH`.

## Windows

A prebuilt, self-contained folder is attached to every release as `stapik-media-<version>-windows-x64.zip` (with a `.sha256` checksum). Unpack it anywhere and run `stapikmedia.exe` — there is no installer. Data is kept per user in `%APPDATA%\stapikmedia` (see the `README.txt` inside the zip for details).

To build it yourself, use an MSYS2 UCRT64 shell with `gcc cmake ninja pkgconf gtkmm-4.0 curl adwaita-icon-theme hicolor-icon-theme` (prefixed `mingw-w64-ucrt-x86_64-`):

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target stapikmedia_windows_bundle
```

The folder appears in `build/stapikmedia-windows/`. Running `stapikmedia.exe --self-test` checks the installation (resources, themes, translations, TLS) and exits with a non-zero code when something is missing; the result is written to `%LOCALAPPDATA%\stapikmedia\cache\stapik.log`.

## Uninstalling

### If installed via `.deb`

```bash
sudo dpkg -r stapikmedia
```

### If installed per-user

```bash
rm -rf ~/.local/opt/stapikmedia
rm ~/.local/bin/stapikmedia
rm ~/.local/share/applications/pl.stapik.media.desktop
rm ~/.local/share/icons/hicolor/256x256/apps/stapikmedia.png
```

Either way, your media data and cloud config remain at `~/.local/share/stapikmedia/`, and your settings at `~/.config/stapikmedia/` — see [Data Storage](#data-storage) below if you want to remove those too.

## Cloud Sync

The app supports synchronization via [Stapik Cloud](https://github.com/Stapik-Group/stapik-cloud) — the same server and protocol used by Stapik Calendar. Go to **File → Connect**, enter the server URL and API key. If a connection is already configured, the app reconnects automatically on startup. Plain HTTP is accepted, but the connection dialog warns that it is not secure.

Synchronization runs in the background and never blocks the window. A status indicator at the bottom shows whether the data is up to date, being synchronized, offline, in conflict or failed. You can trigger a sync manually from **File → Sync**.

Once connected, the app compares the local file and the cloud copy using a `lastUpdate` timestamp and keeps whichever one is newer, overwriting the other **as a whole document**. There is no field-level or entry-level merging — if both copies changed since the last sync, the older one is fully replaced. Every change is saved locally first and pushed to the cloud shortly afterwards (changes made in quick succession are batched). Writes are optimistic-concurrency-checked: if another device saved a newer version in the meantime, the server's copy is fetched and the conflict is resolved by the same rule. If the cloud is unreachable, the change stays saved locally and the app retries with a growing delay. Closing the window right after an edit waits briefly for the pending upload.

### Yearly archives

To keep the synchronized document small, entries are split by the **year they were consumed**:

- entries from the current year (and later) live in the main cloud document `media.json`;
- entries from earlier years are archived, one document per year, as cloud partitions.

At startup only the main document and the archives you already have locally are synchronized. Selecting an older year in the filter downloads that year's archive from the cloud if it is not on this device yet. Until it arrives the list for that year may look empty for a moment.

If you try to add or move an entry into an archived year that exists in the cloud but has not been downloaded yet, the app asks you to select that year in the filter first, so a partial local copy can never overwrite the cloud archive.

When a new year starts, the previous year is archived automatically on the next launch. The same happens once after upgrading from a version that kept all years in a single document.

**Caution for multi-device use:** since conflict resolution is whole-document, editing the same year offline on two different machines before either one reconnects can still cause one set of changes to be discarded. Stapik Cloud keeps a version history of every write, so a discarded document isn't gone permanently, but the app itself doesn't yet expose a way to browse or restore old versions. After upgrading to a version with yearly archives, update the app on all your devices: older versions only know the single-document layout and will not see archived years.

The app talks to the Stapik Cloud `/documents/{slotKey}` endpoint (slot key `media.json`), authenticated via an `x-api-key` header; archives use the same endpoint with a partition per year. See the [Stapik Cloud API reference](https://github.com/Stapik-Group/stapik-cloud) for details.

## Data Storage

Local data is stored in `~/.local/share/stapikmedia/`:

- `media.json` — the main document (current year), written atomically and wrapped with a schema version and a `lastUpdate` timestamp;
- `media-YYYY.json` — one archive per earlier year, in the same format;
- `*.bak` — the three most recent backups of each document;
- `config.json` — the cloud connection.

Language, theme, window size and the last selected category are kept in `~/.config/stapikmedia/settings.json`.

A damaged data file is moved aside instead of being overwritten, and the app starts with an empty list that you can restore from a backup or from the cloud. A file written by a newer version of the app is never modified: the app shows a message and stays read-only.

If you are upgrading from an older version, your existing `media.json` is converted automatically on first launch (the original is kept as a `.bak` file) and every entry gets a stable ID.

## Themes

Switch between five themes from **Settings → Theme**: Classic (the original retro look), Classic Pink, Modern (flat, minimal), Dark, and Neoclassic (a soft blue-and-beige look). The choice applies instantly and is remembered between launches.

![Screenshot](screenshots/screenshot_2.png)
![Screenshot](screenshots/screenshot_3.png)

## TODO

- [x] Category-based sidebar with per-type entry fields
- [x] Filtering by consumption year/month
- [x] Pagination
- [x] Cloud sync with conflict resolution
- [x] `.deb` package for easier distribution
- [x] Multiple themes (Classic / Modern / Classic Pink / Dark / Neoclassic)
- [x] Undo / Redo
- [x] Yearly archives in the cloud
- [x] Unit tests
- [ ] Import from an existing spreadsheet (CSV/XLSX)
- [ ] Entry search by title
- [ ] Sort options (by title, release date, consumption date)
- [ ] Flatpak package