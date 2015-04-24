// --- BEGIN Global variables -----------------------------------
// --- END Global variables -------------------------------------

// Returns the current backed up board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no current backed up board exists.
function currentBackedupBoard() {
    var tr = $("#table_bboards tr.selectedRow:first");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current backed up board name (as a string) or undefined if no current backed up board exists.
function currentBackedupBoardName() {
    var tr = currentBackedupBoard();
    return tr ? tr.find("td:first").text() : undefined;
}

// Returns the current backed up board ID or undefined if no current backed up board exists.
function currentBackedupBoardID() {
    var tr = currentBackedupBoard();
    return $(tr).data("bid");
}

// Returns the first backed up board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no backed up boards exist.
function firstBackedupBoard() {
    tr = $("#table_bboards tr.tr_bboards:nth-child(1)");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current live board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no current live board exists.
function currentLiveBoard() {
    var tr = $("#table_lboards tr.selectedRow:first");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current live board name (as a string) or undefined if no current live board exists.
function currentLiveBoardName() {
    var tr = currentLiveBoard();
    return tr ? tr.find("td:first").text() : undefined;
}

// Returns the current live board ID or undefined if no current live board exists.
function currentLiveBoardID() {
    var tr = currentLiveBoard();
    return $(tr).data("bid");
}

// Returns the first live board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no live boards exist.
function firstLiveBoard() {
    tr = $("#table_lboards tr.tr_lboards:nth-child(1)");
    return (tr.length > 0) ? tr : undefined;
}

// Retrieves the available backed up boards.
function getBackedupBoards() {
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
                        html += "<td>" + boards[i].name + "</td>";
                        html += "<td>" + boards[i].id + "</td>";
                        html += "<td>" + formatUnixUTCTimestamp(boards[i].last_ct) + "</td>";
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

// Retrieves the open live boards.
function getLiveBoards() {
    statusBase = "getting live boards ...";
    updateLiveStatus(statusBase, true);

    query = "?cmd=get_live_boards&filter=" + $("#lboard_name_filter").val();
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateLiveStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

                    // load table
                    clearTable("#table_lboards");
		    boards = data.boards;
                    html = "";
                    for (i = 0; i < boards.length; ++i) {
			var id = boards[i].id;
                        html += "<tr class=\"tr_lboards\" id=\"tr_lb_" + i + "\">";
                        html += "<td>" + boards[i].name + "</td>";
                        html += "<td>" + id + "</td>";
                        html += "<td id=owner_" + id + " style=\"color:red\">pending...</td>";
                        html += "<td id=page_" + id + " style=\"color:red\">pending...</td>";
                        html += "</tr>";
                    }

                    $("#table_lboards > tbody:last").append(html);
                    $("#table_lboards").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_lboards").trigger("appendCache");

                    for (i = 0; i < boards.length; ++i)
			$("#tr_lb_" + i).data("bid", boards[i].id);

                    setCurrentLiveBoard(firstLiveBoard());

		    // complete table by getting summary for each board
                    for (i = 0; i < boards.length; ++i)
			getLiveBoardSummary(boards[i].id);
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Retrieves info for a given live board.
function getLiveBoardSummary(board_id) {
    statusBase = "getting info for board " + board_id + " ...";
    updateLiveStatus(statusBase, true);

    query = "?cmd=get_live_board_summary&id=" + board_id;
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateLiveStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

                    // insert summary in table
		    $('#owner_' + board_id).html(data.owner).css('color', '');
		    $('#page_' + board_id).html('<a href=\"' + data.url + '\">link</a>').css('color', '');
                    $("#table_lboards").trigger("update");
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Opens the HTML snapshot of the current backed up board in a new page.
function showHtmlOfCurrentBackedupBoard() {
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

// Opens the HTML snapshot of the current live board in a new page.
function showHtmlOfCurrentLiveBoard() {
    statusBase = "getting HTML of current live board ...";
    updateLiveStatus(statusBase, true);

    query = "?cmd=get_live_board_html&id=" + currentLiveBoardID();
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    // NOTE: the window to display the HTML must be opened already at this point
    // (and not after the asynchronous server response), otherwise it may be considered
    // suspicious by the popup blocker
    var newTitle = currentLiveBoardName();
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
                        updateLiveStatus(statusBase + " failed: " + data.error, false);
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

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
            updateLiveStatus(statusBase + " error: " + descr, false);
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Backs up the current live board.
function backupCurrentLiveBoard() {
    statusBase = "backing up current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentLiveBoardName();
    $('#backup_status').html('backing up live board <u>' + boardName + '</u> ...').css('color', '');

    query = "?cmd=backup_board&id=" + currentLiveBoardID();
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateLiveStatus(statusBase + " failed: " + data.error, false);
			$('#backup_status').html('error: ' + data.error).css('color', 'red');
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

		    if (data.commit == '') {
			$('#backup_status').html('no changes in live board <u>' + boardName + '</u>');
		    } else {
			$('#backup_status').html(
			    'backed up new changes in live board <u>' + boardName +
				'</u> to local git repository (commit id: ' + data.commit + ' )');
			getBackedupBoards(); // refresh
		    }
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
	    $('#backup_status').html('error: ' + descr).css('color', 'red');
        }

        // complete: function(request, textStatus) {
        //     alert("complete; request.status: " + request.status)
        // }

    });

    return false;
}

// Copies the current live board.
function copyCurrentLiveBoard() {

    if ($('#copy_button').attr('disabled'))
	return;

    $('#copy_button').attr('disabled', true);
    statusBase = "copying current live board ...";
    updateLiveStatus(statusBase, true);
    var srcBoardName = currentLiveBoardName();
    var dstBoardName = $("#dst_board_name").val();
    $('#copy_status').html('copying live board <u>' + srcBoardName + '</u> to new live board <u>' + dstBoardName + '</u> ...')
	.css('color', '');

    query = "?cmd=copy_live_board&src_id=" + currentLiveBoardID() + "&dst_name=" + dstBoardName;
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateLiveStatus(statusBase + " failed: " + data.error, false);
			$('#copy_status').html('error: ' + data.error).css('color', 'red');
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

		    $('#copy_status').html(data.status);
		    getLiveBoards(); // refresh
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
	    $('#copy_status').html('error: ' + descr).css('color', 'red');
        },

        complete: function(request, textStatus) {
	    $('#copy_button').attr('disabled', false);
        }
    });

    return false;
}

// Adds missing members to the current live board (useful for adding back non-admin members who left the board by accident!)
function addMissingMembers() {

    if ($('#addmembers_button').attr('disabled'))
	return;

    $('#addmembers_button').attr('disabled', true);
    statusBase = "adding missing members to the current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentLiveBoardName();
    var boardID = currentLiveBoardID();
    $('#addmembers_status').html('adding missing members to board <u>' + boardName + '</u> (' + boardID + ') ...').css('color', '');

    query = "?cmd=add_org_members_to_board&id=" + boardID;
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    $.ajax({
        url: url,
        type: "GET",
        dataType: "json",

        success: function(data, textStatus, request) {
            if (request.readyState == 4) {
                if (request.status == 200) {

                    if (data.error != null) {
                        updateLiveStatus(statusBase + " failed: " + data.error, false);
			$('#addmembers_status').html('error: ' + data.error).css('color', 'red');
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

		    $('#addmembers_status').html(data.status);
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
	    $('#addmembers_status').html('error: ' + descr).css('color', 'red');
        },

        complete: function(request, textStatus) {
	    $('#addmembers_button').attr('disabled', false);
        }
    });

    return false;
}

// Sets given backed up board as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentBackedupBoard(tr) {
    if (tr.length == 0) return;
    $("#table_bboards tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
    $("#curr_bboard").html(currentBackedupBoardName() + " (" + currentBackedupBoardID() + ")");
}

// Handles selecting a row in the table of backed up boards.
// tr is the jQuery selector for the corresponding <tr> element.
function selectBackedupBoard(tr) {
    setCurrentBackedupBoard(tr);
}

// Sets given live board as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentLiveBoard(tr) {
    if (tr.length == 0) return;
    $("#table_lboards tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
    $("#curr_lboard").html(currentLiveBoardName() + " (" + currentLiveBoardID() + ")");
}

// Handles selecting a row in the table of live boards.
// tr is the jQuery selector for the corresponding <tr> element.
function selectLiveBoard(tr) {
    setCurrentLiveBoard(tr);
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
        selectBackedupBoard($(e.target).parent());
    });

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_lboards';
    options.headers = {
	1: { // board ID (random hash, so sorting makes no sense)
	    sorter: false
	},
	3: { // page (all cells contain the text 'link', so sorting makes no sense)
	    sorter: false
	}
    }
    $("#table_lboards").tablesorter(options);
    $(document).on("click", ".tr_lboards td", function(e) {
        selectLiveBoard($(e.target).parent());
    });

    $('#bboard_name_filter').keyup(function (e) {
	if (e.keyCode === 13) {
	    getBackedupBoards();
	}
    });

    $('#lboard_name_filter').keyup(function (e) {
	if (e.keyCode === 13) {
	    getLiveBoards();
	}
    });

    // window.onbeforeunload = function() {
    //     return "Really reload the page?";
    // }

    getBackedupBoards();
    getLiveBoards();
});
