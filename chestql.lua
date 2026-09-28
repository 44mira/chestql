-- lua code for computercraft computer

local ADDRESS = "localhost:4499"

function serializeCSV(inventory)
	local inv_size = inventory.size()
	local inv_list = inventory.list()
	local result_csv = ""

	for slot = 1, inv_size do
		local row = inv_list[slot]
		local csv_row = ""

		if row then
			csv_row = ("%d,%s,%d\n"):format(slot, row.name, row.count)
			result_csv = result_csv .. csv_row
		end
	end

	return result_csv
end

function send_csv(address, body)
	local headers = { ["Content-Type"] = "text/csv" }

	print("Sending to http://" .. address .. "...")
	return http.post("http://" .. address, body, headers)
end

function main()
	local inv = peripheral.find("minecraft:chest")

	local body = serializeCSV(inv)
	local resp = send_csv(ADDRESS, body)

	if resp == nil then
		print("Unsuccessful send.")
	else
		print("Send successful.")
	end
end

main()
