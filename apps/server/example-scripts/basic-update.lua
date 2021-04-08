

function Update() --Run every server tick
    canbus.setAnalogInput(0, counter)
    counter = (counter + 0.001) % 100
end

function Run() --Run once
    
end

--Run on each created instance of script
--Use to initialise global variables
counter = 0