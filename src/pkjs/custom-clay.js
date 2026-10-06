module.exports = function(minified) {
  var clayConfig = this;

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {    
    var minuteSlider = clayConfig.getItemByMessageKey('MinuteHandLength');
    var hourSlider = clayConfig.getItemByMessageKey('HourHandLength');
    minuteSlider.on('change', function() { hourSlider.set(Math.min(minuteSlider.get(), hourSlider.get())); });
    hourSlider.on('change', function() { hourSlider.set(Math.min(minuteSlider.get(), hourSlider.get())); });
    
    clayConfig.getItemById('reset-button').on('click', function() {
      function reset(messageKey) {
        var item = clayConfig.getItemByMessageKey(messageKey);
        item.set(item.config.defaultValue);
      }
      
      reset('ShowDate');
      reset('Font');
      reset('PrimaryColor');
      reset('SecondaryColor');
      reset('TertiaryColor');
      reset('BackgroundColor');
      reset('MinuteHandLength');
      reset('HourHandLength');
      reset('RecurseScale');
      reset('PrimaryHandWidth');
      reset('NotchInset');
      reset('HourMarkers');
      reset('NotchSquircle');
      reset('ShowGizmos');
      reset('DebugSpeed');
      reset('DebugGrid');
    });
  });
};
