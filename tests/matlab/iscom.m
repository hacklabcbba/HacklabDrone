classdef iscom < handle
    %UNTITLED4 Summary of this class goes here
    %   Detailed explanation goes here
    
    properties (Dependent)
        Size@uint16 scalar
    end
    
    properties(Access = public)
        IdMajor@uint8 scalar
        IdMinor@uint8 scalar
        Content@uint8 vector
    end
    
    methods
        function this = iscom(~)
            this.IdMajor = 0;
            this.IdMinor = 0;
            this.Content = uint8([]);
        end
        function value = get.Size(this)
            value = numel(this.Content);
        end
        function SetContent(this, content)
            assert(isnumeric(content), 'Value must be ''numeric vector''.');
            this.Content = typecast(content, 'uint8');
        end
        function SetIdentifier(this, identifier)
            assert(isnumeric(identifier) && (numel(identifier) == 2), 'Value must be ''numeric vector of size 2''.');
            this.IdMajor = identifier(1);
            this.IdMinor = identifier(2);
        end
        function message = Pack(this)
            content = this.Content;
            messageSize = typecast(uint16(numel(content) + 4), 'uint8');
            identifier = [this.IdMajor, this.IdMinor];
            header = [messageSize, identifier];
            message = [header, content];
        end
        function rest = UnPack(this, data)
            assert(isa(data, 'uint8'), 'Value must be ''uint8 vector''.');
            messageSize = typecast(data(1:2), 'uint16');
            contentSize = messageSize - 4;
            this.IdMajor = data(3);
            this.IdMinor = data(4);
            this.Content = data(5:4 + contentSize);
            rest = data(5 + contentSize:end);
        end
    end
end
