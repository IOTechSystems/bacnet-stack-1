


function Update()
    
end

function Run()
    canbus.setAnalogInput(0, 1337.00)
    canbus.setAnalogOutput(0, 1337.00, 1)
    canbus.setAnalogValue(0, 1337.00, 1)

    canbus.setBinaryInput(0, 1)
    canbus.setBinaryOutput(0, 1 )
    canbus.setBinaryValue(0, 0)

    canbus.setIntegerValue(0, -1337, 1)
    canbus.setPositiveIntegerValue(0, 4201337, 1)
    canbus.setAccumulatorValue(0, 2222)
end
