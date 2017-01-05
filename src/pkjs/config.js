module.exports = [
  {
    "type": "heading",
    "defaultValue": "Rainpower Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Weather Powered by Dark Sky"
      },
      {
        "type": "input",
        "messageKey": "DARK_SKY_API_KEY",
        "label": "API Key"
      },
      {
        "type": "select",
        "messageKey": "DARK_SKY_REQUEST_UNITS",
        "defaultValue": "auto",
        "label": "Units",
        "options": [
          {
            "label": "Automatic (local)",
            "value": "auto"
          },
          {
            "label": "US (Imperial)",
            "value": "us"
          },
          {
            "label": "SI (Metric)",
            "value": "si"
          }
          // See https://darksky.net/dev/docs/forecast -
          // since we only display temperature,
          // we don't include the "UK" or "Canada" options
          // since they'd just be redundant with "SI" above
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save"
  }
];
