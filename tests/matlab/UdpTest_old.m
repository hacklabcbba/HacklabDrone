% u = udp('169.254.1.1', 5000, 'LocalPort', 5002);
% fopen(u);

packet.sync = uint8(['s' 'l']);
packet.header.length = uint16(0);
packet.header.portID = uint16(1);
packet.data = [];

packetArray = struct2cell(packet);
[s, ~] = size(packetArray);
packetRaw = [];
for i=1:s
    if isstruct(packetArray{i})
        packetRaw = [packetRaw typecast(struct2array(packetArray{i}), 'uint8')];
    else
        packetRaw = [packetRaw typecast(packetArray{i}, 'uint8')];
    end
end

InitChecksum = hex2dec('ffffffff');
crcGen = crc32();
crcGen.Init(InitChecksum);
crcGen.Calc(packetRaw(3:end));
checksum = typecast(crcGen.Checksum, 'uint8');
packetRaw = [packetRaw checksum]

payload = uint8([1:10]);
message = slink();
message.InitMessage(1)
message.LoadPayload(payload)
message.EndMessage()
message.GetPacket()

% fwrite(u, packetRaw , 'uint8')
% fclose(u);
