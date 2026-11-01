Skill Assist - src folder (Geode v5.10.1)

1) Drop the src/ folder into your mod repo (replace the template's src/).
2) CMakeLists.txt:
     set(CMAKE_CXX_STANDARD 23)        <- Geode v5 needs C++23
     file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
3) mod.json: "geode": "5.10.1" and the "gd" versions from the v5 template.
   Dependencies, if you add any, use the v5 object format.

src/Preset  - Top 500 / Top 100 / Top 1 + custom settings, saved between launches
src/Assist  - SkillEngine (look-ahead sim + frame reliability) and ClickDelay (input hooks + delay queue)
src/Menu    - SkillMenu popup (v5 Popup), SkillIcon (draggable pause icon), MenuHooks (main menu button + pause hook)
