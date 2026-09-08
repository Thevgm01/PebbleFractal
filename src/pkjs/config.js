module.exports = [
  {
    "type": "heading",
    "defaultValue": "Analog Fractal - Settings"
  },
  {
    "type": "text",
    "defaultValue": "Customize the watchface's appearance."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Date"
      },
      {
        "type": "toggle",
        "messageKey": "ShowDate",
        "label": "Show Date",
        "defaultValue": true
      },
      {
        "type": "select",
        "messageKey": "Font",
        "defaultValue": 2,
        "label": "Date Font",
        "description": "Bigger fonts may have positioning issues when the fractal covers a large portion of the screen",
        "options": [
          { "label": "Gothic 14",            "value": 0 },
          { "label": "Gothic 14 Bold",       "value": 1 },
          { "label": "Gothic 18",            "value": 2 },
          { "label": "Gothic 18 Bold",       "value": 3 },
          { "label": "Gothic 24",            "value": 4 },
          { "label": "Gothic 24 Bold",       "value": 5 },
          { "label": "Gothic 28",            "value": 6 },
          { "label": "Gothic 28 Bold",       "value": 7 },
          { "label": "Roboto Condensed 21",  "value": 13 },
          { "label": "Leco 20 Bold",         "value": 16 },
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "PrimaryColor",
        "defaultValue": "0xFFFFFF",
        "label": "Primary Color"
      },
      {
        "type": "color",
        "messageKey": "SecondaryColor",
        "defaultValue": "0xbbbbbb",
        "label": "Secondary Color"
      },
      {
        "type": "color",
        "messageKey": "TertiaryColor",
        "defaultValue": "0x444444",
        "label": "Tertiary Color"
      },
      {
        "type": "color",
        "messageKey": "BackgroundColor",
        "defaultValue": "0x000000",
        "label": "Background Color"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Sizing"
      },
      {
        "type": "slider",
        "messageKey": "MinuteHandLength",
        "label": "Minute Hand Length",
        "defaultValue": 40,
        "min": 0,
        "max": 100,
        "step": 5
      },
      {
        "type": "slider",
        "messageKey": "HourHandLength",
        "label": "Hour Hand Length",
        "defaultValue": 30,
        "description": "Must not be longer than the minute hand",
        "min": 0,
        "max": 100,
        "step": 5
      },
      {
        "type": "slider",
        "messageKey": "RecurseScale",
        "label": "Recurse Scale",
        "defaultValue": 0.85,
        "min": 0.5,
        "max": 1,
        "step": 0.01
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Legibility"
      },
      {
        "type": "toggle",
        "messageKey": "ShowGizmos",
        "label": "Show Minute/Hour Symbols",
        "defaultValue": false
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Debug"
      },
      {
        "type": "toggle",
        "messageKey": "DebugSpeed",
        "defaultValue": false,
        "label": "Fastmode",
        "description": "Time passes 60x faster"
      },
      {
        "type": "toggle",
        "messageKey": "DebugGrid",
        "defaultValue": false,
        "label": "Show Grid",
        "description": "Show the grid used to calculate date placement"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
