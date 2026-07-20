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
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Sizes"
      },
      {
        "type": "select",
        "messageKey": "FontSize",
        "defaultValue": "18",
        "label": "Font Size",
        "options": [
          { 
            "label": "14",
            "value": "14" 
          },
          { 
            "label": "18",
            "value": "18" 
          },
          { 
            "label": "24",
            "value": "24" 
          }
        ]
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
        "min": 0,
        "max": 100,
        "step": 5
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
        "description": "Optionally make the hands thicker towards teh top",
        "min": 0,
        "max": 1,
        "step": 0.05
      },
      {
        "type": "slider",
        "messageKey": "FirstHandScale",
        "label": "First Hand Scale",
        "defaultValue": 1,
        "description": "If this is greater than 1, make the topmost hands extra long to help with legibility",
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
