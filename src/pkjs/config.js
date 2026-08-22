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
        "defaultValue": "0x444444",
        "label": "Secondary Color"
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
    "description": "Bigger fonts may have display issues when the fractal covers a large portion of the screen",
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
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Sizes"
      },
      {
        "type": "slider",
        "messageKey": "MinuteHandLength",
        "label": "Minute Hand Length",
        "defaultValue": 60,
        "min": 0,
        "max": 100,
        "step": 5
      },
      {
        "type": "slider",
        "messageKey": "HourHandLength",
        "label": "Hour Hand Length",
        "defaultValue": 40,
        "description": "Must not be longer than the minute hand",
        "min": 0,
        "max": 100,
        "step": 5
      },
      {
        "type": "slider",
        "messageKey": "RecurseScale",
        "label": "Recurse Scale",
        "defaultValue": 0.8,
        "min": 0.5,
        "max": 1,
        "step": 0.01
      },
      {
        "type": "slider",
        "messageKey": "FirstHandScale",
        "label": "Top Hand Scale",
        "defaultValue": 1,
        "description": "Lengthen the topmost hands to help with legibility (does not affect the fractal)",
        "min": 1,
        "max": 3,
        "step": 0.25
      },
      {
        "type": "slider",
        "messageKey": "WidthScale",
        "label": "Width Scale",
        "defaultValue": 0,
        "description": "Experimental - Make the hands appear \"thicker\" towards the root",
        "min": 0,
        "max": 0.5,
        "step": 0.05
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
