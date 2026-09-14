# Craft Tracker (SKSE + Prisma UI)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Skyrim SE / AE](https://img.shields.io/badge/Skyrim-SE%20%7C%20AE-brightgreen.svg)]()
[![Prisma UI](https://img.shields.io/badge/UI-Prisma%20UI-orange.svg)](https://www.prismaui.dev/)

A modern, persistent in-game HUD widget for **The Elder Scrolls V: Skyrim Special Edition / Anniversary Edition** that tracks missing crafting ingredients in real-time. Built with **SKSE64** (C++23) and rendered with **Prisma UI (Ultralight)**.

---

## 🇷🇺 Описание на русском

**Craft Tracker** — плагин SKSE с лёгким HTML5-виджетом на базе **Prisma UI**, позволяющий отслеживать необходимые материалы для крафта оружия, брони, алхимии и кузнечного дела прямо на экране во время игры.

### Возможности:
1. **Закрепление рецептов прямо из меню крафта**:
   - Наведите курсор на желаемый рецепт в меню крафта и нажмите **`F`** (настраивается в INI) для закрепления/открепления.
   - Поддерживаются любые рецепты кузницы, верстака, точила, дубильного станка и алхимии.
2. **Множитель крафта (Craft Multiplier)**:
   - Нажмите **`Shift + F`** на рецепте, чтобы циклично переключать количество: **`x1 → x2 → x3 → x5 → x10 → x25 → x50`**.
   - Требуемые ингредиенты автоматически умножаются.
   - При создании каждой единицы предмета множитель уменьшается (`x5 → x4 → ...`), а рецепт автоматически открепляется, когда скрафчена последняя штука.
3. **Быстрое скрытие/показ виджета (Toggle HUD)**:
   - В любой момент вне меню нажмите **`H`**, чтобы мгновенно скрыть или показать виджет (удобно для скриншотов).
4. **Persistent HUD-виджет**:
   - Элегантная полупрозрачная карточка в верхнем левом углу экрана.
   - Индикация прогресса сбора ингредиентов: `имеется / требуется` (например, `2 / 5`).
   - Собранные ингредиенты вычёркиваются и подсвечиваются зелёным.
   - Когда все материалы собраны — карточка подсвечивается зелёным со статусом **«✔ ГОТОВО К КРАФТУ»**.
5. **Реактивное обновление инвентаря без просадки FPS**:
   - Подписка на событие `TESContainerChangedEvent` гарантирует моментальный пересчет без периодических тяжелых опросов инвентаря.
6. **Сохранение в SKSE Cosave**:
   - Закрепленные рецепты привязаны к вашему сохранению через интерфейс сериализации SKSE (ключ `'CRFT'`). При смене порядка загрузки модов FormID автоматически разрешаются через `ResolveFormID`.
7. **Безопасная отрисовка через Prisma UI**:
   - Чистый HTML5/CSS/JS через Ultralight, нулевая нагрузка на движок игры, отсутствие устаревшего Flash/Scaleform.

---

## 🇬🇧 English Description

**Craft Tracker** is an SKSE plugin featuring a sleek, responsive HTML5 HUD widget powered by **Prisma UI** that lets you pin and track missing materials for smithing, alchemy, and crafting directly on your screen.

### Features:
- **Pin / Unpin directly in crafting menus**: Press **`F`** (customizable) to pin or unpin any item from the crafting menu.
- **Craft Multiplier**: Press **`Shift + F`** to cycle multipliers: **`x1 → x2 → x3 → x5 → x10 → x25 → x50`**. The tracker counts down with each craft and unpins once the batch is done.
- **HUD Toggle**: Press **`H`** at any time during gameplay to toggle widget visibility.
- **Real-time Inventory Tracking**: Hooks natively into container change events — instantly updates when you loot, buy, drop, or craft items.
- **Persistent Cosave**: Pinned recipes and progress are saved in your savegame cosave.
- **Zero FPS impact**: Modern C++23 native backend with lightweight Ultralight web UI rendering.

---

## 📋 Требования / Requirements

* **The Elder Scrolls V: Skyrim Special Edition / Anniversary Edition** (1.5.97 / 1.6.x)
* **[SKSE64](https://skse.silverlock.org/)**
* **[Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)**
* **[Prisma UI](https://www.nexusmods.com/skyrimspecialedition/mods/148718)** (v1.4+)
  * 🌐 Официальный сайт: [https://www.prismaui.dev/](https://www.prismaui.dev/)
  * 📦 Nexus Mods: [https://www.nexusmods.com/skyrimspecialedition/mods/148718](https://www.nexusmods.com/skyrimspecialedition/mods/148718)

---

## 🎮 Совместимость / Compatibility

Плагин скомпилирован на базе **CommonLibSSE-ng** с поддержкой **Address Library**, что обеспечивает полную кросс-версионность (один и тот же DLL-файл работает везде):

- ✅ **Skyrim Anniversary Edition (AE)**: `1.6.1130` / `1.6.1170` (Steam), `1.6.1179` (GOG) — **полная поддержка последней версии AE!**
- ✅ **Skyrim Anniversary Edition (AE Legacy)**: `1.6.318` – `1.6.640`
- ✅ **Skyrim Special Edition (SE)**: `1.5.97`
- ✅ **Skyrim VR** (при наличии VR Address Library)
- ✅ Совместим с любыми сборками и модлистами (**RFAB**, **Nolvus**, **LoreRim**, **Elysium Remastered** и др.)
- ✅ Совместим с модами на кастомные интерфейсы (**Dear Diary**, **Nordic UI**, **TrueHUD**, **Compass Navigation Overhaul** и др.)

---

## 💾 Установка / Installation

### Через менеджер модов (рекомендуется):
1. Установите **Prisma UI** и его зависимости.
2. Установите архив **Craft Tracker** через **Mod Organizer 2** или **Vortex**.
3. Убедитесь, что плагин активирован в вашем менеджере модов.

### Вручную (Manual):
Распакуйте содержимое архива в папку `Skyrim Special Edition/Data/`:
```
Data/
├── SKSE/
│   └── Plugins/
│       ├── CraftTracker.dll
│       └── CraftTracker.ini
└── PrismaUI/
    └── views/
        └── CraftTracker/
            ├── index.html
            └── hammer.png
```

---

## ⚙️ Настройки / Configuration (`CraftTracker.ini`)

Файл настроек располагается в `Data/SKSE/Plugins/CraftTracker.ini` (создается автоматически):
```ini
[General]
; Скан-код клавиши (DirectX scan code) для закрепления рецепта в меню крафта:
; 33 (0x21) = F, 20 (0x14) = T, 37 (0x25) = K
iHotkey = 33

; Скан-код клавиши для быстрого скрытия/показа HUD вне меню (0 = отключено):
; 35 (0x23) = H
iToggleHudHotkey = 35

; Показывать уведомление на экране при закреплении/готовности
bShowNotifications = true

; Воспроизводить звуки интерфейса Скайрима
bPlaySounds = true

; Автоматически скрывать виджет, когда скрыт ванильный HUD (меню паузы, диалоги, катсцены)
bHideWhenHudHidden = true
```

---

## 🛠️ Сборка из исходников / Building from Source

### Необходимые инструменты:
- Visual Studio 2022 (MSVC v143 с поддержкой C++23)
- [xmake](https://xmake.io/) (версия 2.8+)
- [CommonLibSSE-ng](https://github.com/CharmedBaryon/CommonLibSSE-ng)

### Инструкция:
```bash
# Клонируйте репозиторий
git clone https://github.com/PerfLite/CraftTracker.git
cd CraftTracker

# Подключите CommonLibSSE-ng в папку lib/commonlibsse
git clone https://github.com/CharmedBaryon/CommonLibSSE-ng.git lib/commonlibsse

# Сборка Release
xmake build -y
```
Собранная DLL будет находиться в `build/windows/x64/release/CraftTracker.dll`.

---

## 🤝 Благодарности и ссылки / Credits & Links

- **[Prisma UI](https://www.prismaui.dev/)** — фреймворк пользовательского интерфейса для Skyrim SE/AE на базе Ultralight HTML5:
  - 🌐 Официальный сайт: [https://www.prismaui.dev/](https://www.prismaui.dev/)
  - 📦 Страница на Nexus Mods: [https://www.nexusmods.com/skyrimspecialedition/mods/148718](https://www.nexusmods.com/skyrimspecialedition/mods/148718)
- **[CommonLibSSE-ng](https://github.com/CharmedBaryon/CommonLibSSE-NG)** — кросс-версионная библиотека SKSE от CharmedBaryon.
- **[CommonLibSSE](https://github.com/Ryan-rsm-McKenzie/CommonLibSSE)** — оригинальная библиотека CommonLibSSE от Ryan (Ryan-rsm-McKenzie).
- **[SKSE Team](https://skse.silverlock.org/)** — Ian Patterson, Stephen Abel, Paul Connelly, Brendan Borthwick.

---

## 📄 Лицензия / License

Проект распространяется под лицензией **GNU General Public License v3.0 (GPL-3.0)**.  
Подробности смотрите в файле [LICENSE](LICENSE).