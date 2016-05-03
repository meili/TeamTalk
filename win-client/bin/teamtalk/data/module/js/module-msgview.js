/**
 * @desc 客户端消息显示模块
 * @author buji
 * @time 2013-08-27
 */
var IMApp = IMApp || {};
IMApp.MessageView = (function($, _) {
    // message content dom
    var $msgview = $('#msgview');
    
    var $doc = $(document);
    
    var hisClicked = false;

    var clientWidth = document.documentElement.clientWidth; // 可视窗口高度
    var clientHeight = document.documentElement.clientHeight; // 可视窗口高度
        
    $(window).resize(function() {
        clientWidth = document.documentElement.clientWidth; // 可视窗口高度
        clientHeight = document.documentElement.clientHeight; // 可视窗口高度
    
        $msgview.css({
            'width': clientWidth,
            'height': clientHeight
        });
    }).resize();
    
    // 新消息显示
    var sendMessage = function(message) {
        message = JSON.parse(message);
        var msgtype = message.msgtype;
        var template = _.template($('#tpl_message' + msgtype).html());
        var $dom = template(message);

        var needScroll = true;
        
        if (Math.abs(clientHeight + $msgview.scrollTop() - $msgview[0].scrollHeight) > 100) {
            needScroll = false;
        }
        
        $msgview.append($dom);
        
        if (needScroll) {
            setTimeout(function() {
                $msgview.scrollTop(1000000);
            }, 0);
        }
    };

    // 历史消息显示
    var historyMessage = function(messages) {
        var message, msgtype, template, $dom, len;

        messages = JSON.parse(messages);

        $dom = '';
        len = messages.length;

        for (var i = 0; i < len; i++) {
            message = messages[i];
            msgtype = message.msgtype;
            template = _.template($('#tpl_message' + msgtype).html());
            $dom += template(message);
        }

        if (hisClicked) {
            $dom += '<p style="height:5px;margin:0 20px 20px 20px;border-bottom:2px dotted #ccc;">&nbsp;</p>';
        }

        $($dom).insertAfter($('#im_history_btn'));
        if (len < 10) {
            $('#im_history_btn').hide();
        }
        if (len > 0) {
            $('#hisline').show();
        }
        $msgview.scrollTop(0);
        
        hisClicked = true;
    };
    
    var scrollBottom = function(){
        $msgview.scrollTop(1000000);
    };
    
    var initEvent = function() {
        var Class_focus = 'message_focus';

        $('#msgview')
            .on('click', '.J_voicebtn', function() {
                var voiceid = $(this).attr('data-id');
                IMApp.MessageView.playVoice(voiceid);
            })
            .on('click', '.message', function() {
                var $this = $(this);

                if ($(this).hasClass(Class_focus)) return;

                $('.' + Class_focus).removeClass(Class_focus);
                $this.addClass(Class_focus);
            });
    };

    return {
        init: function() {
            initEvent();
        },
        /**
         * 发送消息
         * @param message {string} 消息对象，json格式
         *          |- mtype      {string} 发送人   ['me': 自己 'other': 对方]
         *          |- name       {string} 用户昵称
         *          |- avatar     {string} 用户头像
         *          |- time       {string} 消息发送时间
         *          |- src        {string} 图片src
         *          |- width      {string} 图片宽度
         *          |- height     {string} 图片高度
         *          |- goodsTitle {string} 商品名称
         *          |- goodsPrice {string} 商品价格
         *          |- goodsPicurl{string} 商品图片
         *          |- goodsUrl   {string} 商品链接
         *          |- goodsMsgId {string} 带商品信息id
         *          |- height     {string} 图片高度
         *          |- filepath   {string} 文件路径
         *          |- filename   {string} 文件名
         *          |- filesize   {string} 文件大小
         *          |- fileid     {string} 文件id
         *          |- msgtype    {string} 消息类型 [1: 文本消息 2: 图片消息 3: 商品消息 4: 信息提示 5: 文件提示 6: 语音]
         *          |- voiceid    {string} 语音id
         *          |- voicetime  {string} 语音时长
         *          |- voiceisread{string} 语音是否已读 'true' | 'false'
         *          `- content    {string} 消息文本
         */
        send: function(message) {
            sendMessage(message);
        },
        /**
         * 历史消息
         * @param message {array} 消息数组
         */
        history: function(messages) {
            historyMessage(messages);
        },
        scrollBot:function(){
            scrollBottom();
        },
        /**
         * 播放声音
         * @param message {string} 语音消息id
         */
        playVoice: function(voiceid) {
            var $voicebtn = $('.J_voicebtn').filter('[data-id=' + voiceid + ']');
            $voicebtn.addClass('message_voice_play');
            $voicebtn.parent().find('.unread').removeClass('unread');
        },
        /**
         * 停止播放
         * @param message {string} 语音消息id
         */
        stopVoice: function(voiceid) {
            $('.J_voicebtn').filter('[data-id=' + voiceid + ']').removeClass('message_voice_play');
        },
        /**
         * 显示商品信息
         * @param goodsinfo {object} 商品信息
         *  {
                goodsTitle: '2013新款欧美简约女装百搭时尚娃娃领镶钻无袖雪纺衫小衫背心上衣',
                goodsPrice: '89.00',
                goodsPicurl: 'http://s12.mogujie.cn/b7/bao/130819/q22xe_kqyuuq3ikfbg2rdwgfjeg5sckzsew_400x600.jpg_100x100.jpg',
                goodsUrl: "http://www.mogujie.com",
                goodsMsgId: '1x1x1x'
            }
         */
        showGoodsInfo: function(goodsinfo) {
            goodsinfo = JSON.parse(goodsinfo);
        
            if (goodsinfo && goodsinfo['goodsMsgId']) {
                var template = _.template($('#tpl_goodsinfo').html());
                $('.J_goodsinfo').filter('[data-id=' + goodsinfo['goodsMsgId'] + ']').html(template(goodsinfo));
            }            
        }
    };
}(jQuery, window._));

window.sendMessage = IMApp.MessageView.send;
window.scrollBottom = IMApp.MessageView.scrollBot;
window.historyMessage = IMApp.MessageView.history;
window.stopVoice = IMApp.MessageView.stopVoice;
window.playVoice = IMApp.MessageView.playVoice;
window.showGoodsInfo = IMApp.MessageView.showGoodsInfo;

/*
    showGoodsInfo(JSON.stringify({
        goodsTitle: '2013新款欧美简约女装百搭时尚娃娃领镶钻无袖雪纺衫小衫背心上衣',
        goodsPrice: '89.00',
        goodsPicurl: 'http://s12.mogujie.cn/b7/bao/130819/q22xe_kqyuuq3ikfbg2rdwgfjeg5sckzsew_400x600.jpg_100x100.jpg',
        goodsUrl: "http://www.mogujie.com",
        goodsMsgId: '1x1x1x'
    }))
*/

$(document).ready(function() {
    IMApp.MessageView.init();
	window.open("moguim://moguim/:documentcompleted");
});
