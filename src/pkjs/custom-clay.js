module.exports = function(minified) {
  var clayConfig = this;

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function() {
    var minuteSlider = clayConfig.getItemByMessageKey('MinuteHandLength');
    var hourSlider = clayConfig.getItemByMessageKey('HourHandLength');
    
    minuteSlider.on('change', function() { hourSlider.set(Math.min(minuteSlider.get(), hourSlider.get())); });
    hourSlider.on('change', function() { hourSlider.set(Math.min(minuteSlider.get(), hourSlider.get())); });
  });
};
