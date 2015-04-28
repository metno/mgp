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




// Returns the current open live board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no current open live board exists.
function currentOpenLiveBoard() {
    var tr = $("#table_lboards_open tr.selectedRow:first");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current open live board name (as a string) or undefined if no current open live board exists.
function currentOpenLiveBoardName() {
    var tr = currentOpenLiveBoard();
    return tr ? tr.find("td:first").text() : undefined;
}

// Returns the current open live board ID or undefined if no current open live board exists.
function currentOpenLiveBoardID() {
    var tr = currentOpenLiveBoard();
    return $(tr).data("bid");
}

// Returns the first open live board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no open live boards exist.
function firstOpenLiveBoard() {
    tr = $("#table_lboards_open tr.tr_lboards_open:nth-child(1)");
    return (tr.length > 0) ? tr : undefined;
}




// Returns the current closed live board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no current closed live board exists.
function currentClosedLiveBoard() {
    var tr = $("#table_lboards_closed tr.selectedRow:first");
    return (tr.length > 0) ? tr : undefined;
}

// Returns the current closed live board name (as a string) or undefined if no current live closed board exists.
function currentClosedLiveBoardName() {
    var tr = currentClosedLiveBoard();
    return tr ? tr.find("td:first").text() : undefined;
}

// Returns the current closed live board ID or undefined if no current closed live board exists.
function currentClosedLiveBoardID() {
    var tr = currentClosedLiveBoard();
    return $(tr).data("bid");
}

// Returns the first closed live board (as a jQuery selector of the corresponding <tr> element)
// or undefined if no closed live boards exist.
function firstClosedLiveBoard() {
    tr = $("#table_lboards_closed tr.tr_lboards_closed:nth-child(1)");
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

// Retrieves the open live boards.
function getOpenLiveBoards() {
    statusBase = "getting open live boards ...";
    updateLiveStatus(statusBase, true);

    query = "?cmd=get_live_boards&open=1&filter=" + $("#lboard_open_name_filter").val();
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
                    clearTable("#table_lboards_open");
		    boards = data.boards;
                    html = "";
                    for (i = 0; i < boards.length; ++i) {
			var id = boards[i].id;
                        html += "<tr class=\"tr_lboards_open\" id=\"tr_lbo_" + i + "\">";
                        html += "<td style=\"width:1%; white-space:nowrap; padding:5px 10px\">" + boards[i].name + "</td>";
                        html += "<td id=page_" + id + " style=\"color:red; width:1%; padding:5px 10px\">pending...</td>";
                        html += "<td style=\"width:1%; padding:5px 10px\">" + id + "</td>";
                        html += "<td id=owners_" + id +
			    " style=\"color:red; white-space:nowrap; padding:5px 10px\">pending...</td>";
                        html += "</tr>";
                    }

                    $("#table_lboards_open > tbody:last").append(html);
                    $("#table_lboards_open").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_lboards_open").trigger("appendCache");

                    for (i = 0; i < boards.length; ++i)
			$("#tr_lbo_" + i).data("bid", boards[i].id);

                    setCurrentOpenLiveBoard(firstOpenLiveBoard());

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

// Retrieves the closed live boards.
function getClosedLiveBoards() {
    statusBase = "getting closed live boards ...";
    updateLiveStatus(statusBase, true);

    query = "?cmd=get_live_boards&open=0&filter=" + $("#lboard_closed_name_filter").val();
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
                    clearTable("#table_lboards_closed");
		    boards = data.boards;
                    html = "";
                    for (i = 0; i < boards.length; ++i) {
			var id = boards[i].id;
                        html += "<tr class=\"tr_lboards_closed\" id=\"tr_lbc_" + i + "\">";
                        html += "<td style=\"width:1%; white-space:nowrap; padding:5px 10px\">" + boards[i].name + "</td>";
                        html += "<td style=\"padding:5px 10px\">" + id + "</td>";
                        html += "</tr>";
                    }

                    $("#table_lboards_closed > tbody:last").append(html);
                    $("#table_lboards_closed").trigger("update");
                    if (html != "") // hm ... why is this test necessary?
                        $("#table_lboards_closed").trigger("appendCache");

                    for (i = 0; i < boards.length; ++i)
			$("#tr_lbc_" + i).data("bid", boards[i].id);

                    setCurrentClosedLiveBoard(firstClosedLiveBoard());
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

// Retrieves summary of a given live board.
function getLiveBoardSummary(board_id) {
    statusBase = "getting summary of board " + board_id + " ...";
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
		    var html = '';
		    $.each(data.owners, function(index, value) {
			if (value == 'metorg_adm')
			    html += ('<span style="color:green; font-weight:bold">' + value + '</span> ');
			else
			    html += ('<span style="color:#555">' + value + '</span> ');
		    });
		    // ### consider removing the row if non-admin users have promoted themselves as owner!

		    $('#owners_' + board_id).html(html).css('color', '');
		    $('#page_' + board_id).html('<a href=\"' + data.url + '\">link</a>').css('color', '');
                    $("#table_lboards_open").trigger("update");
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

// Opens the HTML snapshot of the current open live board in a new page.
function showHtmlOfCurrentOpenLiveBoard() {
    statusBase = "getting HTML of current open live board ...";
    updateLiveStatus(statusBase, true);

    query = "?cmd=get_live_board_html&id=" + currentOpenLiveBoardID();
    url = "http://" + location.host + "/cgi-bin/metorgtrello" + query;

    // NOTE: the window to display the HTML must be opened already at this point
    // (and not after the asynchronous server response), otherwise it may be considered
    // suspicious by the popup blocker
    var newTitle = currentOpenLiveBoardName();
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

// Backs up the current open live board.
function backupCurrentLiveBoard() {
    statusBase = "backing up current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentOpenLiveBoardName();
    $('#backup_status').html('backing up live board <u>' + boardName + '</u> ...').css('color', '');

    query = "?cmd=backup_board&id=" + currentOpenLiveBoardID();
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

// Copies the current open live board.
function copyCurrentOpenLiveBoard() {

    if ($('#copy_button').attr('disabled'))
	return;

    $('#copy_button').attr('disabled', true);
    statusBase = "copying current live board ...";
    updateLiveStatus(statusBase, true);
    var srcBoardName = currentOpenLiveBoardName();
    var dstBoardName = $("#copy_dst_board_name").val();
    $('#copy_status').html('copying live board <u>' + srcBoardName + '</u> to new live board <u>' + dstBoardName + '</u> ...')
	.css('color', '');

    query = "?cmd=copy_live_board&src_id=" + currentOpenLiveBoardID() + "&dst_name=" + dstBoardName;
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
		    getOpenLiveBoards(); // refresh
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

// Renames the current open live board.
function renameCurrentOpenLiveBoard() {

    if ($('#rename_button').attr('disabled'))
	return;

    $('#rename_button').attr('disabled', true);
    statusBase = "renaming current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentOpenLiveBoardName();
    var newName = $("#rename_new_board_name").val();
    $('#rename_status').html('renaming live board <u>' + boardName + '</u> to <u>' + newName + '</u> ...')
	.css('color', '');

    query = "?cmd=rename_live_board&id=" + currentOpenLiveBoardID() + "&new_name=" + newName;
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
			$('#rename_status').html('error: ' + data.error).css('color', 'red');
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

		    $('#rename_status').html(data.status);
		    getOpenLiveBoards(); // refresh
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
	    $('#rename_status').html('error: ' + descr).css('color', 'red');
        },

        complete: function(request, textStatus) {
	    $('#rename_button').attr('disabled', false);
        }
    });

    return false;
}

// Adds missing members to the current open live board (useful for adding back non-admin members who left the board by accident!)
function addMissingMembers() {

    if ($('#addmembers_button').attr('disabled'))
	return;

    $('#addmembers_button').attr('disabled', true);
    statusBase = "adding missing members to the current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentOpenLiveBoardName();
    var boardID = currentOpenLiveBoardID();
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

// Closes the current open live board.
// ### Very similar to reopenCurrentLiveBoard(), consider refactoring.
function closeCurrentLiveBoard() {

    if ($('#close_button').attr('disabled'))
	return;

    $('#close_button').attr('disabled', true);
    statusBase = "closing the current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentOpenLiveBoardName();
    var boardID = currentOpenLiveBoardID();
    $('#close_status').html('closing board <u>' + boardName + '</u> (' + boardID + ') ...').css('color', '');

    query = "?cmd=close_board&id=" + boardID;
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
			$('#close_status').html('error: ' + data.error).css('color', 'red');
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

		    $('#close_status').html(data.status);
		    getOpenLiveBoards(); // refresh
		    getClosedLiveBoards(); // refresh
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
	    $('#close_status').html('error: ' + descr).css('color', 'red');
        },

        complete: function(request, textStatus) {
	    $('#close_button').attr('disabled', false);
        }
    });

    return false;
}

// Reopens the current open live board.
// ### Very similar to closeCurrentLiveBoard(), consider refactoring.
function reopenCurrentLiveBoard() {

    if ($('#reopen_button').attr('disabled'))
	return;

    $('#reopen_button').attr('disabled', true);
    statusBase = "reopening the current live board ...";
    updateLiveStatus(statusBase, true);
    var boardName = currentClosedLiveBoardName();
    var boardID = currentClosedLiveBoardID();
    $('#reopen_status').html('reopening board <u>' + boardName + '</u> (' + boardID + ') ...').css('color', '');

    query = "?cmd=reopen_board&id=" + boardID;
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
			$('#reopen_status').html('error: ' + data.error).css('color', 'red');
                        return
                    }

                    updateLiveStatus(statusBase + " done", false);
                    updateLiveStatus("", false);

		    $('#reopen_status').html(data.status);
		    getOpenLiveBoards(); // refresh
		    getClosedLiveBoards(); // refresh
                }
            }
        },

        error: function(request, textStatus, errorThrown) {
            descr = errorThrown;
            if (errorThrown == null) {
                descr = "undefined error - is the server down?";
            }
            updateLiveStatus(statusBase + " error: " + descr, false);
	    $('#reopen_status').html('error: ' + descr).css('color', 'red');
        },

        complete: function(request, textStatus) {
	    $('#reopen_button').attr('disabled', false);
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

// Sets given open live board as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentOpenLiveBoard(tr) {
    if (tr.length == 0) return;
    $("#table_lboards_open tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
    $("#curr_lboard_open").html(currentOpenLiveBoardName() + " (" + currentOpenLiveBoardID() + ")");
}

// Handles selecting a row in the table of open live boards.
// tr is the jQuery selector for the corresponding <tr> element.
function selectOpenLiveBoard(tr) {
    setCurrentOpenLiveBoard(tr);
}

// Sets given closed live board as current.
// tr is the jQuery selector for the corresponding <tr> element.
function setCurrentClosedLiveBoard(tr) {
    if (tr.length == 0) return;
    $("#table_lboards_closed tr").removeClass("selectedRow"); // unselect all rows
    tr.addClass("selectedRow"); // select target row
    $("#curr_lboard_closed").html(currentClosedLiveBoardName() + " (" + currentClosedLiveBoardID() + ")");
}

// Handles selecting a row in the table of closed live boards.
// tr is the jQuery selector for the corresponding <tr> element.
function selectClosedLiveBoard(tr) {
    setCurrentClosedLiveBoard(tr);
}

function selectBoardType() {
    var type = $('#board_type').val();
    if (type == 'open') {
	$('#div_lboards_open').css('display', 'block')
	$('#div_lboards_closed').css('display', 'none')
	$('#board_type').css('color', 'green');
    } else {
	$('#div_lboards_open').css('display', 'none')
	$('#div_lboards_closed').css('display', 'block')
	$('#board_type').css('color', 'red');
    }
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

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
	}
    });


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_lboards_open';

    options.headers = {
	1: { // board ID (random hash, so sorting makes no sense)
	    sorter: false
	},
	2: { // page (all cells contain the text 'link', so sorting makes no sense)
	    sorter: false
	}
    }

    $("#table_lboards_open").tablesorter(options);

    $(document).on("click", ".tr_lboards_open td", function(e) {
        selectOpenLiveBoard($(e.target).closest('tr'));
    });

    $(document).on("click", ".tr_lboards_open span", function(e) {
        selectOpenLiveBoard($(e.target).closest('tr'));
    });

    $('#lboard_open_name_filter').keyup(function (e) {
	if (e.keyCode === 13) {
	    getOpenLiveBoards();
	}
    });


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    options.widgetOptions.stickyHeaders_attachTo = '.wrapper_lboards_closed';

    options.headers = {
	1: { // board ID (random hash, so sorting makes no sense)
	    sorter: false
	}
    }

    $("#table_lboards_closed").tablesorter(options);

    $(document).on("click", ".tr_lboards_closed td", function(e) {
        selectClosedLiveBoard($(e.target).closest('tr'));
    });

    $('#lboard_closed_name_filter').keyup(function (e) {
	if (e.keyCode === 13) {
	    getClosedLiveBoards();
	}
    });

    // window.onbeforeunload = function() {
    //     return "Really reload the page?";
    // }

    selectBoardType();
    getBackedupBoards();
    getOpenLiveBoards();
    getClosedLiveBoards();
});
