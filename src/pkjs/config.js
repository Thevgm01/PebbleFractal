module.exports = [
  {
    "type": "heading",
    "defaultValue": "Fractal Settings"
  },
  {
    "type": "text",
    "defaultValue": "Customize your watchface appearance and preferences."
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
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Sizes"
      },
      {
        "type": "slider",
        "messageKey": "RecurseScale",
        "label": "Recurse Scale",
        "defaultValue": 0.9,
        "min": 0,
        "max": 1,
        "step": 0.05
      },
      {
        "type": "slider",
        "messageKey": "WidthScale",
        "label": "Width Scale",
        "defaultValue": 0,
        "min": 0,
        "max": 1,
        "step": 0.05
      },
      {
        "type": "slider",
        "messageKey": "TrueHandScale",
        "label": "Recurse Scale",
        "defaultValue": 2,
        "min": 1,
        "max": 3,
        "step": 0.25
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
