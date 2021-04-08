

function Update() --Run every server tick
    
end

function Run() --Run once
    bacnet.setAnalogInput(0, 1337.00)
    bacnet.setAnalogOutput(0, 1337.00, 1)
    bacnet.setAnalogValue(0, 1337.00, 1)

    bacnet.setBinaryInput(0, 1)
    bacnet.setBinaryOutput(0, 1 )
    bacnet.setBinaryValue(0, 0)

    bacnet.setIntegerValue(0, -1337, 1)
    bacnet.setPositiveIntegerValue(0, 4201337, 1)
    bacnet.setAccumulatorValue(0, 2222)
end
