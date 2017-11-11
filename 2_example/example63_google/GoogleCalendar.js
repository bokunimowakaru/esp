function doGet(){
  return ContentService.createTextOutput(GetEvents());
}

function GetEvents(){
  var Now = new Date();
  var EndTime = new Date(Now.getTime() + (6 * 60 * 60 * 1000));
  Logger.log("Start " + Now);
  Logger.log("End " + EndTime);
  var events = CalendarApp.getEvents(Now, EndTime);
  Logger.log(events.length + " Events");
  str = "Length," + events.length + '\n';
  for (var i = 0; i < events.length; i++){
    str += events[i].getStartTime().getHours() + ',';
    str += events[i].getStartTime().getMinutes() + ',';
    str += events[i].getTitle();
    str += '\n';
  }
  str += "[EOF]";
  Logger.log("retrun str\n" + str);
  return str;
}
