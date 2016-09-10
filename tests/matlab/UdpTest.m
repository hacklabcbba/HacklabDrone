function UdpTest(~)

host = '169.254.1.1';
% host = '127.0.0.1';
port = 5000;
timeout = 10;

import java.io.*
import java.net.DatagramSocket
import java.net.DatagramPacket
import java.net.InetAddress
addr = InetAddress.getByName(host);

socket = DatagramSocket;
socket.setReuseAddress(1);
socket.setSoTimeout(timeout);
socket.setReceiveBufferSize(10000);

ts = rem(now,1)*24*60*60;
timestamp = time;
timestamp.seconds = floor(ts);
timestamp.fraction = floor(rem(ts,1)*1e6);
ctrl = control;
ctrl.time = timestamp;
ctrl.data(1) = 1500;
ctrl.data(2) = 1000;
ctrl.data(3) = 2000;
ctrl.data(4) = 0;
ctrl.data(5) = 0;
ctrl.data(6) = 0;
ctrl.data(7) = 0;
ctrl.data(8) = 10;
content1 = cell2mat(timestamp.ToCell);
content2 = cell2mat(ctrl.ToCell);

idTimestamp = [hex2dec('0') hex2dec('1')];
idControl = [hex2dec('2') hex2dec('1A')];
identifier = hex2dec('DB');

message(1) = iscom();
message(2) = iscom();
message(1).SetIdentifier(idTimestamp);
message(2).SetIdentifier(idControl);

tic
for ii=1:1000
    message(1).SetContent(content1);
    message(1).Pack;
    
    message(2).SetContent(content2);
    message(2).Pack;
end
toc

payload = uint8([]);
payload = [payload message(1).Pack()];
payload = [payload message(2).Pack()];

packetLength = 500;
messageTX = slink();
messageRX = slink();
udpPacketTX = DatagramPacket(0, 1, addr, port);
udpPacketRX = DatagramPacket(zeros(1, packetLength,'uint8'), packetLength);

tic
for ii=1:1000
    messageTX.InitMessage();
    messageTX.Identifier = identifier;
    messageTX.LoadPayload(payload);
    messageTX.EndMessage();
    try,
        udpPacketTX.setData(messageTX.Packet);
        socket.send(udpPacketTX);
    end
end
toc

msgRX = iscom();
data = struct('gyro',[],'accel',[],'magnet',[]);

plotLength = 500;
bufferRX = uint8([]);
dataAccel = zeros(plotLength,3);
count = 0;

%% Setup figure and plot
% Create figure
fig = figure('Name', 'UDP Data');
set(gca, 'SortMethod', 'depth');
set(gcf,'NumberTitle','off');
set(gcf, 'Renderer', 'zbuffer');
lighting phong;
hold on; grid on;

% Create graphics handles
accelHandle = plot(dataAccel);

%% Setup timer
t = timer('Period', 0.02 ,'ExecutionMode', 'fixedRate');
t.TimerFcn = @(~,~) (drawnow);
start(t);

tic
for ii=1:1000
    try,
        socket.receive(udpPacketRX);
        mssg = udpPacketRX.getData; mssg = typecast(mssg(1:udpPacketRX.getLength), 'uint8');
        bufferRX = [bufferRX; mssg];
    end
    [result, bufferRX] = messageRX.ReceiveMessage(bufferRX);
    if result == 1
        payload = messageRX.Payload;
        while(numel(payload))
            payload = msgRX.UnPack(payload);
            switch msgRX.IdMajor
                case 0,
                case 1,
                    switch msgRX.IdMinor
                        case 26,
                            data.gyro(end+1).time = time(msgRX.Content(1:8));
                            data.gyro(end).data = typecast(msgRX.Content(9:end), 'uint32');
                        case 27,
                            data.accel(end+1).time = time(msgRX.Content(1:8));
                            data.accel(end).data = typecast(msgRX.Content(9:end), 'uint32');
                            
                            % plot
                            tmp = cell2mat({data.accel(max(end-plotLength+1,1):end).data}');
                            dataAccel(end+1-size(tmp,1):end,:) = tmp;
                            accelHandle(1).YData = dataAccel(:,1);
                            accelHandle(2).YData = dataAccel(:,2);
                            accelHandle(3).YData = dataAccel(:,3);
                        case 28,
                            data.magnet(end+1).time = time(msgRX.Content(1:8));
                            data.magnet(end).data = typecast(msgRX.Content(9:end), 'uint32');
                    end
            end
        end
        count = count+1;
    end
end
toc
disp(count)

stop(t);
delete(t);

socket.close;