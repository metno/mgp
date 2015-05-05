// --- BEGIN Global variables -----------------------------------

var cookie;

// --- END Global variables -------------------------------------

// Returns the current backed up board (as a jQuery selector of the corresponding <tr> element)
// or null if no current backed up board exists.
function currentBackedupBoard() {
    var tr = $("#table_bboards tr.selectedRow:first");
    return (tr.length > 0) ? tr : null;
}

// Returns the current backed up board name (as a string) or null if no current backed up board exists.
function currentBackedupBoardName() {
    var tr = currentBackedupBoard();
    return tr ? tr.find("td:first").text() : null;
}

// Returns the current backed up board ID or null if no current backed up board exists.
function currentBackedupBoardID() {
    var tr = currentBackedupBoard();
    return (tr != null) ? $(tr).data("bid") : null;
}

// Returns the first backed up board (as a jQuery selector of the corresponding <tr> element)
// or null if no backed up boards exist.
function firstBackedupBoard() {
    tr = $("#table_bboards tr.tr_bboards:nth-child(1)");
    return (tr.length > 0) ? tr : null;
}


// Retrieves the available backed up boards.
function getBackedupBoards() {
    loadStateFromCookie();

    statusBase = "getting backed up boards ...";
    updateBackupStatus(statusBase, true);

    query = "?cmd=get_backedup_boards&filter=" + $("#bboard_name_filter").val();
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateBackupStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateBackupStatus(statusBase + " done", false);
                    updateBackupStatus("", false);

                    // load table
                    clearTable("#table_bboards");
		    boards = data.boards;
                    html = "";
                    for (i = 0; i < boards.length; ++i) {
                        html += "<tr class=\"tr_bboards\" id=\"tr_bb_" + i + "\">";
                        html += "<td style=\"width:1%; white-space:nowrap; padding:5px 10px\">" + boards[i].name + "</td>";
                        html += "<td style=\"width:1%; padding:5px 10px\">" + boards[i].id + "</td>";
                        html += "<td style=\"padding:5px 10px\">" + formatUnixUTCTimestamp(boards[i].last_ct) + "</td>";
                        html += "</tr>";
                    }

                    $("#table_bboards > tbody:last").append(html);
                    $("#table_bboards").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_bboards").trigger("appendCache");

                    for (i = 0; i < boards.length; ++i) {
			$("#tr_bb_" + i).data("bid", boards[i].id);
		    }

                    setCurrentBackedupBoard(firstBackedupBoard());
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateBackupStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Opens the HTML snapshot of the current backed up board in a new page.
function showCurrentBackedupBoardAsPrintablePage() {
    statusBase = "getting HTML of current backed up board ...";
    updateBackupStatus(statusBase, true);

    query = "?cmd=get_backedup_board_html&id=" + currentBackedupBoardID();
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    // NOTE: the window to display the HTML must be opened already at this point
    // (and not after the asynchronous server response), otherwise it may be considered
    // suspicious by the popup blocker
    var newTitle = currentBackedupBoardName();
    var newWin = window.open('');
    $(newWin.document.body).html(
	'<html><body><h3>generating static HTML for ' + newTitle + '; please wait ...</h3></body></html>');

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateBackupStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateBackupStatus(statusBase + " done", false);
                    updateBackupStatus("", false);

                    $(newWin.document.body).html(data.html);
                    newWin.document.title = newTitle;
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateBackupStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Sets given backed up board as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentBackedupBoard(tr) {
    if (tr == null) {
	$("#curr_bboard").html("none");
	return;
    }

    $("#table_bboards tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row

    $("#curr_bboard").html(currentBackedupBoardName() + " (" + currentBackedupBoardID() + ")");
}

// Handles selecting a row in the table of backed up boards.
// tr is the jQuery selector for the corresponding <tr> element.
function selectBackedupBoard(tr) {
    setCurrentBackedupBoard(tr);
}

function loadStateFromCookie() {
    if (!Cookie.enabled()) return;

    var state = cookie.getDynamicProperties();

    var filter = state['bboard_name_filter'];
    if (filter) {
	$('#bboard_name_filter').val(filter);
    }
}

function saveStateToCookie() {
    if (!Cookie.enabled()) return;
    cookie.setDynamicProperty('bboard_name_filter', $('#bboard_name_filter').val());
    cookie.store();
}

$(document).ready(function() {

    // --- TABLESORTER -----------------------

    initTablesorter();

    var options = {
        //widthFixed : false,
        //showProcessing: true,
        // headerTemplate : '{content} {icon}', // Add icon for jui theme; new in v2.7!

        // widgets: [ 'uitheme', 'zebra', 'stickyHeaders', 'filter' ],
        //widgets: [ 'uitheme', 'zebra', 'stickyHeaders' ],
        widgets: [ 'uitheme', 'stickyHeaders' ],

        widgetOptions: {

            // extra class name added to the sticky header row
            stickyHeaders : '',
            // number or jquery selector targeting the position:fixed element
            stickyHeaders_offset : 0,
            // added to table ID, if it exists
            stickyHeaders_cloneId : '-sticky',
            // trigger "resize" event on headers
            stickyHeaders_addResizeEvent : true,
            // if false and a caption exist, it won't be included in the sticky header
            stickyHeaders_includeCaption : true,
            // The zIndex of the stickyHeaders, allows the user to adjust this to their needs
            stickyHeaders_zIndex : 2,
            // jQuery selector or object to attach sticky header to
            stickyHeaders_attachTo : null,

            // adding zebra striping, using content and default styles
            // - the ui css removes the background from default
            // even and odd class names included for this demo to allow switching themes
            //zebra   : ["ui-widget-content even", "ui-state-default odd"],
            // use uitheme widget to apply default jquery ui (jui) class names
            // see the uitheme demo for more details on how to change the class names
            uitheme : 'jui'
        }
    };

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_bboards';

    options.headers = {
	1: { // board ID (random hash, so sorting makes no sense)
	    sorter: false
	}
    }

    $("#table_bboards").tablesorter(options);

    $(document).on("click", ".tr_bboards td", function(e) {
        selectBackedupBoard($(e.target).closest('tr'));
    });

    $('#bboard_name_filter').keyup(function (e) {
	if (e.keyCode === 13) {
	    getBackedupBoards();
	} else {
	    saveStateToCookie();
	}
    });

    if (Cookie.enabled()) {
        cookie = new Cookie("metorgtrelloadm");
        window.onbeforeunload = function() { saveStateToCookie(); }
    }

    getBackedupBoards();
});
