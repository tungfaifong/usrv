-- Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

function Start()
	logger.debug("call lua Start")
	local id = timer.CreateTimer(1000, function() logger.debug("call lua timer") end)
	return true
end

function Update(interval)
	return false
end

function Stop()
	logger.debug("call lua Stop")
end

function OnConn(net_id, ip, port)
	logger.debug("OnConn net_id:" .. net_id .. " ip:" .. ip .. " port:" .. port)
end

function OnRecv(net_id, data)
	-- logger.debug("OnRecv net_id:" .. net_id .. " data:" .. data)
	server.Send(net_id, data, string.len(data))
end

function OnDisc(net_id)
	logger.debug("OnDisc net_id:" .. net_id)
end
